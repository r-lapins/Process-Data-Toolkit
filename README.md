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

Basic (global statistics):

```bash
./build/debug/pdt_cli --in data.csv
```

Filter by sensor:

```bash
./build/debug/pdt_cli \
  --in data.csv \
  --sensor S1 \
  --from 2026-02-18T10:00:00 \
  --to   2026-02-18T12:00:00
```

Per-sensor statistics (mutually exclusive with `--sensor`):

```bash
./build/debug/pdt_cli \
  --in data.csv \
  --per-sensor \
  --from 2026-02-18T10:00:00 \
  --to   2026-02-18T12:00:00
```

Export JSON report:

```bash
./build/debug/pdt_cli \
  --in data.csv \
  --per-sensor \
  --out report.json
```

---

## JSON Output (example)

```json
{
  "mode": "per_sensor",
  "import": {
    "parsed_ok": 31,
    "skipped": 7
  },
  "stats_by_sensor": {
    "S1": {
      "count": 14,
      "min": -2,
      "max": 100,
      "mean": 19.45,
      "stddev": 22.9087
    },
    "S2": {
      "count": 10,
      "min": 0,
      "max": 55,
      "mean": 21.3,
      "stddev": 12.1
    }
  }
}
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
- Time filtering is inclusive (`--from`, `--to`)

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
- Per-sensor statistics mode (`--per-sensor`)
- JSON report export (`--out`)
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

- CSV report export
- Simple anomaly detection (z-score)
- Benchmark target