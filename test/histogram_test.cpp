#include <cstdint>

#include "check.hpp"
#include "tsclat/histogram.hpp"

static bool within(std::uint64_t value, std::uint64_t expected, double rel) {
    const double lo = static_cast<double>(expected) * (1.0 - rel);
    const double hi = static_cast<double>(expected) * (1.0 + rel);
    return static_cast<double>(value) >= lo && static_cast<double>(value) <= hi;
}

int main() {
    using tsclat::Histogram;

    {
        Histogram h;
        for (std::uint64_t v = 0; v < 1000; ++v) {
            h.record(v);
        }
        CHECK(h.count() == 1000);
        CHECK(h.max() == 999);
        CHECK(within(h.value_at_quantile(0.50), 500, 0.05));
        CHECK(within(h.value_at_quantile(0.90), 900, 0.05));
        CHECK(within(h.value_at_quantile(0.99), 990, 0.05));
    }

    {
        // Small values are counted exactly.
        Histogram h;
        h.record(7);
        h.record(42);
        CHECK(h.value_at_quantile(0.0) == 7);
        CHECK(h.max() == 42);
    }

    {
        // Heavy tail: 99% at 100, 1% at 1e6 — the p99.9 must land in the tail.
        Histogram h;
        for (int i = 0; i < 9900; ++i) {
            h.record(100);
        }
        for (int i = 0; i < 100; ++i) {
            h.record(1'000'000);
        }
        CHECK(within(h.value_at_quantile(0.50), 100, 0.05));
        CHECK(within(h.value_at_quantile(0.999), 1'000'000, 0.05));
        CHECK(h.mean() > 100.0);
    }

    RUN_END();
}
