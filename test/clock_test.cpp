#include <chrono>
#include <cstdint>

#include "check.hpp"
#include "tsclat/clock.hpp"

int main() {
    using namespace tsclat;
    using sc = std::chrono::steady_clock;

    const TscClock& clock = TscClock::instance();
    CHECK(clock.ns_per_tick() > 0.02);
    CHECK(clock.ns_per_tick() < 5.0);

    (void)has_invariant_tsc();

    const auto wall_start = sc::now();
    const std::uint64_t tsc_start = tsc_begin();
    const auto deadline = wall_start + std::chrono::milliseconds(50);
    while (sc::now() < deadline) {
    }
    const std::uint64_t tsc_stop = tsc_end();
    const double wall_ns = std::chrono::duration<double, std::nano>(sc::now() - wall_start).count();
    const double tsc_ns = clock.to_ns(tsc_stop - tsc_start);

    const double ratio = tsc_ns / wall_ns;
    CHECK(ratio > 0.9);
    CHECK(ratio < 1.1);

    RUN_END();
}
