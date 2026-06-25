#pragma once

#include <algorithm>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <utility>
#include <vector>

#include "tsclat/clock.hpp"
#include "tsclat/histogram.hpp"

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

struct Report {
    std::string name;
    std::uint64_t samples;
    double overhead_ns;
    double min_ns;
    double p50_ns;
    double p90_ns;
    double p99_ns;
    double p999_ns;
    double p9999_ns;
    double max_ns;
    double mean_ns;
};

template <typename F>
Report bench(std::string name, int warmup, int iters, F&& body,
             std::uint64_t expected_interval_ns = 0) {
    const TscClock& clock = TscClock::instance();
    const double floor_ns = measure_overhead(iters < 20000 ? 20000 : iters).min_ns;

    for (int i = 0; i < warmup; ++i) {
        body();
    }

    Histogram hist;
    for (int i = 0; i < iters; ++i) {
        const std::uint64_t t0 = tsc_begin();
        body();
        const std::uint64_t t1 = tsc_end();
        double ns = clock.to_ns(t1 - t0) - floor_ns;
        if (ns < 0.0) {
            ns = 0.0;
        }
        const std::uint64_t sample = static_cast<std::uint64_t>(ns + 0.5);
        if (expected_interval_ns > 0) {
            hist.record_corrected(sample, expected_interval_ns);
        } else {
            hist.record(sample);
        }
    }

    return Report{std::move(name),
                  hist.count(),
                  floor_ns,
                  static_cast<double>(hist.value_at_quantile(0.0)),
                  static_cast<double>(hist.value_at_quantile(0.50)),
                  static_cast<double>(hist.value_at_quantile(0.90)),
                  static_cast<double>(hist.value_at_quantile(0.99)),
                  static_cast<double>(hist.value_at_quantile(0.999)),
                  static_cast<double>(hist.value_at_quantile(0.9999)),
                  static_cast<double>(hist.max()),
                  hist.mean()};
}

inline void print_report(const Report& r) {
    std::printf("%-24s  n=%llu  overhead=%.1f ns\n", r.name.c_str(),
                static_cast<unsigned long long>(r.samples), r.overhead_ns);
    std::printf(
        "  p50 %.0f  p90 %.0f  p99 %.0f  p99.9 %.0f  p99.99 %.0f  max %.0f  mean %.1f  (ns)\n",
        r.p50_ns, r.p90_ns, r.p99_ns, r.p999_ns, r.p9999_ns, r.max_ns, r.mean_ns);
}

inline bool write_csv(const Report& r, const std::string& path) {
    std::ofstream out(path);
    if (!out) {
        return false;
    }
    out << "quantile,ns\n";
    out << "0.5," << r.p50_ns << "\n";
    out << "0.9," << r.p90_ns << "\n";
    out << "0.99," << r.p99_ns << "\n";
    out << "0.999," << r.p999_ns << "\n";
    out << "0.9999," << r.p9999_ns << "\n";
    out << "1.0," << r.max_ns << "\n";
    return true;
}

}  // namespace tsclat
