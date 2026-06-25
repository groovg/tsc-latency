#include <cmath>
#include <cstdint>

#include "tsclat/bench.hpp"

int main() {
    volatile double sink = 0.0;
    double angle = 0.0;

    const tsclat::Report report = tsclat::bench("std::sin", 10000, 1'000'000, [&] {
        angle += 1e-6;
        sink = std::sin(angle);
    });

    tsclat::print_report(report);
    tsclat::write_csv(report, "std_sin.hgrm.csv");
    return 0;
}
