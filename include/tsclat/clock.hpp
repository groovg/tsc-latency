#pragma once

#include <chrono>
#include <cstdint>
#include <cstdio>

#include "tsclat/tsc.hpp"

#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <cpuid.h>
#endif

namespace tsclat {

inline bool has_invariant_tsc() {
#if defined(_MSC_VER)
    int regs[4];
    __cpuid(regs, 0x80000007);
    return (regs[3] & (1 << 8)) != 0;
#else
    unsigned eax, ebx, ecx, edx;
    if (__get_cpuid(0x80000007u, &eax, &ebx, &ecx, &edx) == 0) {
        return false;
    }
    return (edx & (1u << 8)) != 0;
#endif
}

class TscClock {
public:
    static const TscClock& instance() {
        static const TscClock clock = calibrate();
        return clock;
    }

    double ns_per_tick() const { return ns_per_tick_; }
    double to_ns(std::uint64_t ticks) const { return static_cast<double>(ticks) * ns_per_tick_; }
    bool invariant() const { return invariant_; }

private:
    TscClock(double ns_per_tick, bool invariant)
        : ns_per_tick_(ns_per_tick), invariant_(invariant) {}

    static TscClock calibrate(std::chrono::milliseconds window = std::chrono::milliseconds(10)) {
        const bool invariant = has_invariant_tsc();
        if (!invariant) {
            std::fprintf(stderr,
                         "tsclat: invariant TSC not reported by CPUID; calibration may drift\n");
        }
        using clock = std::chrono::steady_clock;
        const auto wall_start = clock::now();
        const std::uint64_t tsc_start = tsc_begin();
        const auto deadline = wall_start + window;
        while (clock::now() < deadline) {
        }
        const std::uint64_t tsc_end_ticks = tsc_end();
        const auto wall_end = clock::now();

        const double ns = std::chrono::duration<double, std::nano>(wall_end - wall_start).count();
        const double ticks = static_cast<double>(tsc_end_ticks - tsc_start);
        return TscClock(ns / ticks, invariant);
    }

    double ns_per_tick_;
    bool invariant_;
};

}  // namespace tsclat
