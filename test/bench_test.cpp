#include <cstdint>

#include "check.hpp"
#include "tsclat/bench.hpp"

int main() {
    const tsclat::Overhead overhead = tsclat::measure_overhead(50000);
    CHECK(overhead.min_ns > 0.0);
    CHECK(overhead.min_ns < 500.0);
    CHECK(overhead.median_ns >= overhead.min_ns);

    std::uint64_t counter = 0;
    const tsclat::Report report = tsclat::bench("increment", 1000, 200000, [&] { ++counter; });
    CHECK(report.samples == 200000);
    CHECK(counter == 201000);
    CHECK(report.p50_ns <= report.p99_ns);
    CHECK(report.p99_ns <= report.max_ns);

    double x = 1.0;
    const tsclat::Report pinned = tsclat::bench("do_not_optimize", 100, 1000, [&] {
        x *= 1.000001;
        tsclat::do_not_optimize(x);
    });
    CHECK(pinned.samples == 1000);
    CHECK(x > 1.0);

    RUN_END();
}
