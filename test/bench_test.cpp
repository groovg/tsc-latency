#include "check.hpp"
#include "tsclat/bench.hpp"

int main() {
    const tsclat::Overhead overhead = tsclat::measure_overhead(50000);
    CHECK(overhead.min_ns > 0.0);
    CHECK(overhead.min_ns < 500.0);
    CHECK(overhead.median_ns >= overhead.min_ns);

    RUN_END();
}
