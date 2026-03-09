# Process Data Toolkit

![CI](https://github.com/r-lapins/Process-Data-Toolkit/actions/workflows/ci.yml/badge.svg)

Modern C++20 library + CLI for processing industrial sensor data and basic signal analysis.

The project combines industrial data processing (CSV time-series) with a small signal-processing module (DFT, spectrum analysis, spectral peak detection).

Designed as a production-style project with a clear domain model, unit tests, sanitizers, static analysis, and CI.

---

## Requirements

- C++20 compatible compiler
- CMake ≥ 3.25
- Ninja

Tested with:

- GCC
- Clang
- Linux

---

## Build

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

---

## Testing

Run all unit tests:

```bash
ctest --preset debug
```

Tests cover:

- CSV parsing
- timestamp parsing
- filtering logic
- statistics computation
- per-sensor statistics
- anomaly detection
- CLI argument parsing
- spectral peak detection

---

## Quick example

Input CSV:

```
timestamp,sensor,value
2026-02-18T10:00:00,S1,1.0
2026-02-18T10:30:00,S2,2.0
2026-02-18T11:00:00,S1,3.0
```

Run:

```bash
./build/debug/pdt_cli --in sample.csv --sensor S1
```

Output:

```
Import:
  parsed_ok: 3
  skipped:   0
Data:
  total:     3
  filtered:  2
Stats:
  count:  2
  min:    1
  max:    3
  mean:   2
  stddev: 1
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

---

## Anomaly detection (z-score)

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
    }
  }
}
```

---

## Input format

CSV with header:

```
timestamp,sensor,value
2026-02-18T10:00:00,S1,1.0
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
- Signal processing utilities (DFT, spectral peaks)
- WAV (PCM16 mono) reader
- Spectrum analysis demo for real signals
- CMake presets (Debug / Release)
- ASAN + UBSAN in Debug builds
- Unit tests
- GitHub Actions CI
- clang-format
- clang-tidy

---

# Signal Processing Module

The project also contains a small experimental module for **time-series and spectral analysis** implemented in modern C++.

Features:

- Discrete Fourier Transform (DFT)
- Single-sided spectrum computation
- Configurable spectral peak detection
- Detection of dominant spectral components
- WAV (PCM16 mono) input support

---

## Spectrum demo

Build the project:

```
cmake --preset debug
cmake --build --preset debug
```

### WAV signal analysis

The demo can also analyze a real WAV file.

Run:

```
./build/debug/spectrum_demo input.wav
```

Example output:

```
Loaded WAV file
sample_rate = 48000 Hz
channels    = 1
samples     = 949760
Analyzed range: [300, 10000)
segment size   = 9700

Dominant peaks
-------------------------------------
f = 7298.97 Hz |X| = 71.7
f = 7417.73 Hz |X| = 61.8
...
```

The demo:

1. loads a WAV file (PCM16 mono)
2. extracts a fragment of samples
3. computes the spectrum
4. detects dominant spectral peaks

---

### Synthetic signal demo

(Change in *app/spectrum_demo.cpp* needed -> build)

Run:

```
./build/debug/spectrum_demo
```

The demo:

1. generates a synthetic signal with multiple sinusoids
2. computes the spectrum
3. detects spectral peaks
4. reports dominant frequencies

Example signal:

x(t) = sin(2π·50t) + 0.5·sin(2π·120t)

Expected dominant frequencies:

50 Hz
120 Hz

---

# Algorithms implemented

### Statistics

```
mean
min
max
stddev
```

Standard deviation:

```
σ = sqrt( Σ(x - μ)² / N )
```

---

### Z-score anomaly detection

```
z = (x - μ) / σ
```

Samples with `|z| > threshold` are reported as anomalies.

---

### Discrete Fourier Transform

```
X[k] = Σ x[n] · e^(−j2πkn/N)
```

Current implementation is `O(N²)` and serves as a reference implementation.

---

### Spectral peak detection

Two strategies:

**ThresholdOnly**

```
X[i] >= threshold_ratio · max(X)
```

**LocalMaxima**

```
X[i] > X[i-1] && X[i] > X[i+1]
```

---

### Dominant spectral components

Dominant frequencies are extracted by:

1. detecting peaks
2. sorting them by magnitude
3. selecting strongest components

---

## Project structure

```
include/pdt/   public API
src/           library implementation
app/           CLI application + demos
tests/         unit tests
.github/       CI workflows
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

## Library usage

Example:

```cpp
#include <pdt/dataset.h>
#include <pdt/csv_reader.h>

#include <fstream>

int main() {
    std::ifstream in("sample.csv");

    auto import = pdt::read_csv(in);

    pdt::DataSet ds{std::move(import.samples)};

    auto stats = ds.stats();

    return 0;
}
```

---

## Design goals

- modern C++20
- clear separation between CLI and core library
- testable domain logic
- reproducible builds via CMake presets
- CI with multiple compilers

---

## Roadmap

- Robust anomaly detection (MAD / IQR)
- Streaming / incremental processing
- Benchmark target
- Larger dataset tests

---

## Development

Development notes and CI instructions are available in [DEVELOPMENT.md](DEVELOPMENT.md).
