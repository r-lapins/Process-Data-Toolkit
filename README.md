# Process Data Toolkit (PDT)

![CI](https://github.com/r-lapins/Process-Data-Toolkit/actions/workflows/ci.yml/badge.svg)

Modern C++20 library and CLI tools for time-series processing and basic signal analysis.

---

## Project goals

This project demonstrates modern C++ development practices and serves as a portfolio example.

Key aspects:

- Modern C++20 design
- Clean separation between CLI and reusable core library
- Reproducible builds using CMake presets
- CI (GCC + Clang)
- Sanitizers (ASan + UBSan)
- Static analysis (clang-tidy)
- Debugging and memory analysis (GDB + Valgrind)
- Unit testing

---

## Features

### Core data processing

- CSV parser with import summary (`parsed_ok`, `skipped`)
- ISO 8601 timestamp parsing using `std::chrono`
- Data filtering by sensor and time range
- Domain model based on `DataSet` class
- Statistical analysis (`count`, `mean`, `min`, `max`, `stddev`)
- Per-sensor statistics mode (`--per-sensor`)
- Z-score based anomaly detection (`--z`, `--top`)
- JSON report export (`--out`)

### Signal analysis

- Discrete Fourier Transform (DFT)
- Radix-2 Fast Fourier Transform (FFT)
- Automatic DFT / FFT selection depending on segment length
- Single-sided spectrum computation
- Window functions: Hann and Hamming
- Spectral peak detection (`ThresholdOnly`, `LocalMaxima`)
- Dominant spectral peak extraction
- WAV reader (RIFF/WAVE PCM16 mono)
- CLI spectrum analysis tool for WAV files (`spectrum_cli`)
- Synthetic signal spectrum analysis demo (`spectrum_synth_demo`)

---

## Project structure

```
include/pdt/          public API
include/pdt/core/     core data processing API
include/pdt/signal/   signal processing API

src/core/             core implementation
src/signal/           signal processing implementation

app/                  CLI applications
tests/                unit tests
examples/             sample CSV and WAV inputs
.github/              CI workflows
```

---

## Requirements

- CMake 3.25+
- Ninja
- C++20 compatible compiler
- Linux environment is recommended

---

## Build

### Debug (sanitizers)

```bash
cmake --preset debug
cmake --build --preset debug
```

### Debug (no sanitizers — for Valgrind/GDB)

```bash
cmake --preset debug-nosan
cmake --build --preset debug-nosan
```

### Release

```bash
cmake --preset release
cmake --build --preset release
```

### Run examples

Build and run the CLI:

```bash
./build/release/pdt_cli --in examples/sample.csv
./build/release/spectrum_cli --in examples/file.wav
```

---

## Testing & Debugging

### Run tests

With sanitizers:

```bash
ctest --preset debug
```

Without sanitizers

```bash
ctest --preset debug-nosan
```

### Debugging (GDB)

Use a debug build to inspect program execution with GDB.

#### Run with GDB

Without sanitizers:

```bash
gdb ./build/debug-nosan/spectrum_cli
```

Example debugging session:

```gdb
break main
run examples/file.wav
next
step
print var
display var
info locals
bt
```

### Memory checking (Valgrind)

Use the `debug-nosan` preset when running Valgrind.

Why `debug-nosan`? 

AddressSanitizer and Valgrind should not be used in the same build configuration.

#### Valgrind Memcheck

```bash
valgrind --leak-check=full --track-origins=yes \
./build/debug-nosan/pdt_cli --in examples/sample.csv \
--per-sensor
```

---

## CLI usage

Basic (global statistics):

```bash
./build/debug/pdt_cli --in examples/sample.csv
```

Filter by sensor and export JSON report:

```bash
./build/debug/pdt_cli \
  --in examples/sample.csv \
  --per-sensor \
  --from 2026-02-18T10:00:00 \
  --to   2026-02-18T12:00:00 \
  --out examples/report.json
```

#### Anomaly detection (z-score)

Detect outliers using z-score threshold:

```bash
./build/debug/pdt_cli \
  --in examples/sample.csv \
  --sensor S1 \
  --z 2.5 \
  --top 5
```

#### JSON Output (example)

```json
{
  "mode": "per_sensor",
  "import": {
    "parsed_ok": 37,
    "skipped": 9
  },
  "filter": {
    "from": "2026-02-18T08:00:00",
    "to": "2026-02-18T18:15:00"
  },
  "data": {
    "total": 37,
    "filtered": 37
  },
  "stats_by_sensor": {
    "S1": {
      "count": 15,
      "min": -50,
      "max": 100,
      "mean": 14.82,
      "stddev": 28.1058
    }
  },
  "anomalies": {
    "method": "zscore",
    "threshold": 1,
    "top_n": 5,
    "mode": "per_sensor",
    "per_sensor": {
      "S1": {
        "count": 1,
        "top": [
          {"timestamp":"2026-02-18T12:45:00","sensor":"S1","value":-50,"z":-2.30629}
        ]
      },
      "S2": { "count": 0, "top": [] }
    }
  }
}

```

#### Input format

CSV with header:

```
timestamp,sensor,value
2026-02-18T10:00:00,S1,1.0
```

Notes:

- Timestamp format: `YYYY-MM-DDTHH:MM:SS`
- Invalid lines are skipped and reported
- Time filtering is inclusive

---

## Signal Processing Module

The project includes a signal processing module for basic spectral analysis implemented in modern C++.

### Pipeline

```
Signal (WAV / synthetic) → segment selection → optional windowing → DFT / FFT → Spectrum → Peak detection → Dominant spectral components
```

### Spectrum CLI (WAV input)

Run:

```
./build/debug/spectrum_cli input.wav
./build/debug/spectrum_cli --in examples/HDSDR_20230515_072359Z_15047kHz_AF.wav
```

Example with explicit options:

```
./build/debug/spectrum_cli \
  --in input.wav \
  --window hann \
  --from 0 \
  --bins 1024 \
  --threshold 0.4 \
  --mode local-maxima \
  --top 10 \
  --algorithm auto \
  --out output_csv
```

Supported options:

```
--in <file.wav>
--window <none|hann|hamming>
--from <index>
--bins <count>
--threshold <0..1>
--mode <threshold-only|local-maxima>
--top <count>
--algorithm <auto|dft|fft>
--out <file.csv>
```

What the spectrum CLI does:

1. Reads a WAV file
2. Decodes PCM16 mono samples
3. Extracts a selected sample range
4. Optionally applies a window function
5. Computes a single-sided spectrum using DFT or FFT
6. Detects spectral peaks
7. Reports dominant spectral peaks

Example output:

```
Input file   : examples/sample.wav
Sample rate  : 48000 Hz
Channels     : 1
Samples      : 949760
From sample  : 4736
Bins         : 512
Window       : hamming
Algorithm    : fft
Threshold    : 0.2
Peak mode    : local-maxima
Top peaks    : 2

Dominant peaks
-------------------------------------
1. f = 7312.5 Hz    |X| = 6.69779    (bin 78)
2. f = 7125 Hz    |X| = 2.93414    (bin 76)
```

The computed spectrum can be exported to CSV for further analysis.

Example:

```
frequency_hz,magnitude
0,0.0001
46.875,0.0032
93.75,0.0211
...
```

The file can be opened in:

- Excel / LibreOffice
- Python (NumPy / Pandas)
- MATLAB

### Spectrum demo (synthetic signal)

The project also includes a small synthetic-signal demo:

```
./build/debug/spectrum_synth_demo
```

The demo:

1. Generates a synthetic signal with multiple sinusoids
2. Computes the spectrum
3. Detects spectral peaks
4. Reports dominant spectral peaks

Example signal:

x(t) = sin(2π·50t) + 0.5·sin(2π·120t)

Expected dominant spectral peaks:

50 Hz
120 Hz

---

## Benchmark

The repository includes a simple benchmark comparing the runtime of the naive Discrete Fourier Transform and the radix-2 Fast Fourier Transform.

Run:

```
./build/release/fft_benchmark
```

Example output:

```
N,DFT(ms),FFT(ms)
64,0.08,0.01
128,0.3,0.01
256,1.16,0.03
512,4.57,0.07
1024,18.02,0.14
2048,71.7,0.32
```

This demonstrates the expected complexity difference:

```
DFT  ~ O(N²)
FFT  ~ O(N log N)
```

---

## Algorithms

#### Standard deviation:

```
σ = sqrt( Σ(x - μ)² / N )
```

#### Z-score anomaly detection

```
z = (x - μ) / σ
```

Samples with `|z| > threshold` are reported as anomalies.

#### Discrete Fourier Transform (DFT)

```
X[k] = Σ x[n] · e^(−j2πkn/N)
```

Current implementation is `O(N²)` and serves as a reference implementation.

#### Spectral peak detection

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

## Library usage

Example:

```cpp
#include <pdt/core/dataset.h>
#include <pdt/core/csv_reader.h>

#include <fstream>

int main() {
    std::ifstream in("examples/sample.csv");

    auto import = pdt::read_csv(in);

    pdt::DataSet ds{std::move(import.samples)};

    auto stats = ds.stats();

    return 0;
}
```

---

## Future work

Possible next steps:

- Additional DSP operations
- Robust anomaly detection (MAD / IQR)
- Streaming processing

---

## Development

Development notes and CI instructions are available in [DEVELOPMENT.md](DEVELOPMENT.md).

---

## License

MIT License
