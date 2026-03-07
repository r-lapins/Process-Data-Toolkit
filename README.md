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
./build/debug/pdt_cli --in sample.csv
```

Filter by sensor:

```bash
./build/debug/pdt_cli \
  --in sample.csv \
  --sensor S1 \
  --from 2026-02-18T10:00:00 \
  --to   2026-02-18T12:00:00
```

Per-sensor statistics (mutually exclusive with `--sensor`):

```bash
./build/debug/pdt_cli \
  --in sample.csv \
  --per-sensor \
  --from 2026-02-18T10:00:00 \
  --to   2026-02-18T12:00:00
```

Export JSON report:

```bash
./build/debug/pdt_cli \
  --in sample.csv \
  --per-sensor \
  --out report.json
```

### Anomaly detection (z-score)

Detect outliers using z-score threshold:

```bash
./build/debug/pdt_cli \
  --in sample.csv \
  --z 3.0
```

Limit number of reported anomalies:

```bash
./build/debug/pdt_cli \
  --in sample.csv \
  --z 2.5 \
  --top 5
```

Per-sensor anomaly detection:

```bash
./build/debug/pdt_cli \
  --in sample.csv \
  --per-sensor \
  --z 2.5
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
  },
  "anomalies": {
    "method": "zscore",
    "threshold": 2.5,
    "top_n": 5,
    "mode": "per_sensor",
    "per_sensor": {
      "S1": {
        "count": 2,
        "top": [
          {
            "timestamp": "2026-02-18T10:45:00",
            "sensor": "S1",
            "value": 100,
            "z": 3.2
          }
        ]
      }
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

Rules:

- Timestamp format: `YYYY-MM-DDTHH:MM:SS`
- Invalid lines are skipped and reported
- Time filtering is inclusive (`--from`, `--to`)
- Duplicate timestamps are allowed

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
- Z-score anomaly detection (`--z`, `--top`)
- JSON report export (`--out`)
- Domain model: `DataSet` encapsulating data and operations
- Separate CLI layer with dedicated argument parser
- CMake presets (Debug / Release)
- ASAN + UBSAN in Debug builds
- Unit tests (core logic + CLI parsing)
- GitHub Actions CI
- clang-format / clang-tidy

---

## Project structure

```
include/pdt/   public API
src/           library implementation
app/           CLI
tests/         unit tests
.github/       CI
```

---

## Architecture

The project is structured around a small domain model.

`DataSet` encapsulates ownership of sensor samples and provides operations such as:

- filtering by sensor and time range
- statistics computation
- per-sensor statistics
- anomaly detection

The CLI layer (`app/`) is intentionally separated from the core library to keep the domain logic reusable and testable.

---

## Roadmap

- Robust anomaly detection (MAD / IQR)
- Streaming / incremental processing
- Benchmark target
- Larger dataset tests

---

## Development

Development notes and CI instructions are available in [DEVELOPMENT.md](DEVELOPMENT.md).