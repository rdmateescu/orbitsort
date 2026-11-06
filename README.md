# OrbitSort

A rotating-pair parallel sort for demonstrating circular compare-exchange sorting.

Pairs orbit the array without colliding, so each phase can process its pairs in parallel.

- Each phase uses `W = floor(n/2)` disjoint pairs `(i, i + 1 mod n)`.
- The wrap-around pair `(n - 1, 0)` is canonicalized as `(0, n - 1)`.
- The rotation offset advances by one position after each phase.
- Sorting stops after two consecutive zero-swap phases for even lengths or three for odd lengths.
- OrbitSort resembles odd-even transposition sort, but uses rotating circular pair layouts and an
  explicit wrap-around pair rather than fixed linear even/odd layouts.

## Logging

OrbitSort supports four logging levels:

- `ORBITSORT_LOG_LEVEL_ERROR`: errors only.
- `ORBITSORT_LOG_LEVEL_IMPORTANT`: errors and important operation messages. This is the default.
- `ORBITSORT_LOG_LEVEL_DEBUG`: adds selected function-boundary messages.
- `ORBITSORT_LOG_LEVEL_TRACE`: enables detailed low-level comparison diagnostics.

Use `set_log_level(ORBITSORT_LOG_LEVEL_TRACE)` when investigating a failure. Normal demonstration
output should use `ORBITSORT_LOG_LEVEL_IMPORTANT` so the console is not overwhelmed by comparison
messages.

## Build

The source currently contains the sorting implementation but no `main` demonstration entry point,
so compile it as an object file:

```bash
gcc -O2 -std=c11 -fopenmp -c orbitsort.c -o orbitsort.o
g++ -O2 -std=c++17 -fopenmp -c orbitsort.cpp -o orbitsort_cpp.o
