#include <cstdio>

#include "tsclat/bench.hpp"

int main() {
    const tsclat::TscClock& clock = tsclat::TscClock::instance();
    std::printf("ns/tick: %.4f   invariant TSC: %s\n", clock.ns_per_tick(),
                clock.invariant() ? "yes" : "no");

    const tsclat::Overhead overhead = tsclat::measure_overhead();
    std::printf("timestamp-pair overhead: min %.1f ns, median %.1f ns\n", overhead.min_ns,
                overhead.median_ns);
    return 0;
}
