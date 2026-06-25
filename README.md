# tsc-latency

[![CI](https://github.com/groovg/tsc-latency/actions/workflows/ci.yml/badge.svg)](https://github.com/groovg/tsc-latency/actions/workflows/ci.yml)

A small, header-only C++20 latency-measurement library: timestamp with the CPU's
invariant TSC, record samples into a log-linear HDR histogram, and report honest
tail percentiles (p50 / p99 / p99.9 / p99.99 / max) — with a coordinated-omission
correction. Means hide tails, and tails are what matter in low-latency systems.

```cpp
#include "tsclat/bench.hpp"

std::uint64_t counter = 0;
auto report = tsclat::bench("increment", /*warmup=*/10'000, /*iters=*/1'000'000,
                            [&] { ++counter; });
tsclat::print_report(report);          // percentile table
tsclat::write_csv(report, "out.csv");  // for plotting
```

```
std::sin                  n=1000000  overhead=10.0 ns
  p50 10  p90 20  p99 20  p99.9 20  p99.99 20  max 2635  mean 11.8  (ns)
```

The harness times each call with `tsc_begin()` / `tsc_end()`, subtracts the
measured timestamp overhead, and records into the histogram. Pass an
`expected_interval_ns` to `bench` to enable the coordinated-omission correction.

## Design

- **Fenced TSC reads.** Plain `RDTSC` is not serializing, so the CPU can reorder it
  around the timed code. `tsc_begin` brackets `RDTSC` with `LFENCE`; `tsc_end` uses
  `RDTSCP` (waits for prior instructions to retire) plus a trailing `LFENCE`.
- **Calibration.** The TSC counts ticks, not nanoseconds. On startup the clock
  calibrates ticks→ns against `steady_clock` over a short window, and checks the
  `invariant TSC` CPUID bit (warning if absent, since calibration would drift).
- **Measurement floor.** Back-to-back timestamps give the cost of measurement
  itself; it's reported and subtracted from samples.
- **Log-linear histogram.** Values below `2^sub_bits` are exact; each octave above
  is split into `2^sub_bits` linear sub-buckets, bounding the relative error across
  the whole `u64` range in a few KB — enough for stable p99.9 / p99.99.
- **Coordinated omission.** When a run stalls, the operations the stall blocked are
  never sampled, which flatters the tail. Given an expected interval, the recorder
  backfills those omitted samples (Gil Tene's correction), so the high percentiles
  reflect what a client would actually have seen.

## Build, test, run

```sh
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure

./build/measure_overhead    # ns/tick, invariant-TSC flag, measurement floor
./build/measure_function    # bench std::sin and dump a percentile table + CSV
```

## Limitations

- x86-64 only (uses `RDTSC`/`RDTSCP`); an ARM `cntvct_el0` backend would slot in
  behind the same API.
- Calibration is only as good as the reference clock and an invariant TSC; pin the
  thread and disable frequency scaling for stable absolute numbers.
- Percentiles are bucketed (bounded relative error), not exact order statistics.

## License

MIT — see [LICENSE](LICENSE).
