#include <cstdint>

#include "check.hpp"
#include "tsclat/tsc.hpp"

int main() {
    using namespace tsclat;

    std::uint64_t prev = tsc_begin();
    for (int i = 0; i < 100000; ++i) {
        const std::uint64_t now = tsc_end();
        CHECK(now >= prev);
        prev = now;
    }

    const std::uint64_t a = tsc_begin();
    std::uint64_t sink = 0;
    for (int i = 0; i < 100000; ++i) {
        sink += static_cast<std::uint64_t>(i);
    }
    const std::uint64_t b = tsc_end();
    CHECK(b > a);
    CHECK(sink > 0);

    RUN_END();
}
