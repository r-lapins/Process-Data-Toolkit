# Process Data Toolkit

Modern C++20 **library + CLI** for processing industrial sensor CSV data.

Designed as a production-style project with a clear domain model, unit tests, sanitizers, and CI.

---

## Build

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

---

## Run

```bash
./build/debug/pdt_cli --in data.csv
```

With filtering:

```bash
./build/debug/pdt_cli \
  --in data.csv \
  --sensor S1 \
  --from 2026-02-18T10:00:00 \
  --to   2026-02-18T12:00:00
```

Test:
```bash
./build/debug/pdt_cli --in sample.csv --sensor S1
```

Output:
```text
Import:
  parsed_ok: 31
  skipped:   7
Data:
  total:     31
  filtered:  14
Stats:
  count:  14
  min:    -2
  max:    100
  mean:   19.45
  stddev: 22.9087
```

---

## Input format

CSV with header:

```text
timestamp,sensor,value
2026-02-18T10:00:00,S1,1.0
2026-02-18T10:30:00,S2,2.0
```

- Timestamp format: `YYYY-MM-DDTHH:MM:SS`
- Invalid lines are skipped and reported

---

## Features

- CSV parsing with import summary (`parsed_ok`, `skipped`)
- ISO 8601 → `std::chrono::sys_seconds`
- Filtering by:
  - sensor (exact match)
  - time range (inclusive)
- Statistics:
  - `count`
  - `mean`
  - `min`
  - `max`
  - `stddev`
- Domain model: `DataSet` encapsulating data and operations
- Separate CLI layer with dedicated argument parser
- CMake presets (Debug / Release)
- ASAN + UBSAN in Debug
- Unit tests (core logic + CLI parsing)
- GitHub Actions CI
- clang-format / clang-tidy

---

## Project structure

```text
include/pdt/   public API
src/           library implementation
app/           CLI
tests/         unit tests
.github/       CI
```

---

## Roadmap

- JSON report export
- CSV report export
- Simple anomaly detection
- Benchmark target
