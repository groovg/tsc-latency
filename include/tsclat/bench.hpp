#pragma once

#include <algorithm>
#include <cstdint>
#include <vector>

#include "tsclat/clock.hpp"

namespace tsclat {

struct Overhead {
    double min_ns;
    double median_ns;
};

inline Overhead measure_overhead(int iters = 100000) {
    std::vector<std::uint64_t> ticks;
    ticks.reserve(static_cast<std::size_t>(iters));
    for (int i = 0; i < iters; ++i) {
        const std::uint64_t a = tsc_begin();
        const std::uint64_t b = tsc_end();
        ticks.push_back(b - a);
    }
    std::sort(ticks.begin(), ticks.end());
    const TscClock& clock = TscClock::instance();
    return {clock.to_ns(ticks.front()), clock.to_ns(ticks[ticks.size() / 2])};
}

}  // namespace tsclat
