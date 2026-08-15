#include <cmath>
#include <cstdint>

#include "tsclat/bench.hpp"

int main() {
    double angle = 0.0;

    const tsclat::Report report = tsclat::bench("std::sin", 10000, 1'000'000, [&] {
        angle += 1e-6;
        double y = std::sin(angle);
        tsclat::do_not_optimize(y);
    });

    tsclat::print_report(report);
    tsclat::write_csv(report, "std_sin.hgrm.csv");
    return 0;
}
