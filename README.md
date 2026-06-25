# tsc-latency

A small, header-only C++20 latency-measurement library: timestamp with the CPU's
invariant TSC, record samples into an HDR histogram, and report honest tail
percentiles (p50 / p99 / p99.9 / p99.99 / max) — including a coordinated-omission
correction.

Means hide tails, and tails are what matter in low-latency systems. This is the
tool the rest of the portfolio uses to back its latency claims.

## Why TSC

`std::chrono::steady_clock` on most platforms resolves to ~100 ns and costs a
function call; the timestamp counter is a single instruction with sub-nanosecond
resolution. The catch is doing it correctly — serialization so the CPU doesn't
reorder the timed region around the read, and calibration to convert ticks to
nanoseconds — which is most of what this library handles.

## Status

Work in progress. See the [docs](#) and `examples/` as they land.

## License

MIT — see [LICENSE](LICENSE).
