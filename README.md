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
                            [&] { ++counter; tsclat::do_not_optimize(counter); });
tsclat::print_report(report);          // percentile table
tsclat::write_csv(report, "out.csv");  // for plotting
```

```
std::sin                  n=1000000  overhead=10.0 ns
  p50 10  p90 20  p99 20  p99.9 20  p99.99 20  max 1443  mean 14.4  (ns)
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
  the whole `u64` range in ~30 KB at the default `sub_bits = 6`. Quantiles are reported
  at the bucket's upper bound (HdrHistogram's highest-equivalent value), clamped to the
  observed max: a coarse bucket can overstate a percentile by up to one bucket width,
  never understate it.
- **Optimizer barrier.** `do_not_optimize(x)` marks a value as observed, so the compiler
  cannot hoist or delete the timed work. Any pure computation inside the lambda needs it;
  the `measure_function` example shows the pattern.
- **Coordinated omission.** When a run stalls, the operations the stall blocked are
  never sampled, which flatters the tail. Given an expected interval, the recorder
  backfills those omitted samples (Gil Tene's correction). This is the post-hoc form; a
  paced open-loop generator that measures from intended start times is the stronger one
  and is not implemented here.

## Build, test, run

```sh
cmake -S . -B build -G Ninja
cmake --build build
ctest --test-dir build --output-on-failure

./build/measure_overhead    # ns/tick, invariant-TSC flag, measurement floor
./build/measure_function    # bench std::sin and dump a percentile table + CSV
```

## What the floor looks like

The numbers above come from a rented dedicated Ryzen 7 9700X (Zen 5, 8 cores on one
CCD), Ubuntu 24.04 / kernel 6.17, gcc 13.3 `-O3`, SMT off, `isolcpus=2-7
nohz_full=2-7 rcu_nocbs=2-7`, `performance` governor, the thread pinned to an
isolated core. The CPU reports an invariant TSC and calibration lands at
0.2637 ns/tick.

| | isolated bare metal (9700X) | desktop (9950X3D, Windows 11, nothing pinned) |
|---|---:|---:|
| timestamp-pair floor, min | 10 ns | 10 ns |
| timestamp-pair floor, median | 20 ns | 20 ns |
| `std::sin` p50 / p99 | 10 / 20 ns | 10 / 20 ns |
| `std::sin` max | 1,443 to 1,703 ns over two runs | 2,635 to 52,971 ns over repeat runs |

The floor is the same on both: a fenced `RDTSC`/`RDTSCP` pair, and it is the limit of what
this library resolves. The maximum differs: a million `std::sin` calls on the desktop
caught stalls between 2.6 and 53 µs across runs, and the same million on an isolated core
stayed under 1.8 µs. OS and CPU also differ and were not separated. A max in the tens of
microseconds over a 20 ns floor is a stall the run caught, not the cost of the call; read
p99.9 and max, and measure on an isolated core.

## Limitations

- x86-64 only (uses `RDTSC`/`RDTSCP`); an ARM `cntvct_el0` backend would slot in
  behind the same API.
- Calibration is only as good as the reference clock and an invariant TSC; pin the
  thread and disable frequency scaling for stable absolute numbers.
- Percentiles are bucketed (bounded relative error), not exact order statistics.
- A 20 ns median floor means sub-100 ns work is measured with a coarse ruler. For
  anything that small, time a batch of N iterations and divide, or accept that the
  low percentiles are quantized to the floor.

## License

MIT — see [LICENSE](LICENSE).
