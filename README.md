# Process Data Toolkit (PDT)

![CI](https://github.com/r-lapins/Process-Data-Toolkit/actions/workflows/ci.yml/badge.svg)

Modern C++20 library and CLI tools for time-series processing and signal analysis.

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

Notes and instructions are available in [docs/CORE.md](docs/CORE.md).

- CSV parser with import summary (`parsed_ok`, `skipped`)
- Optional display of skipped CSV rows with line numbers (`--skipped`)
- ISO 8601 timestamp parsing using `std::chrono`
- Data filtering by sensor and time range
- Domain model based on `DataSet` class
- Statistical analysis (`count`, `mean`, `min`, `max`, `stddev`)
- Per-sensor statistics mode (`--per-sensor`)
- Z-score based anomaly detection (`--z`, `--top`)
- JSON report export (`--out`)

### Signal analysis

Notes and instructions are available in [docs/SIGNAL.md](docs/SIGNAL.md).

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
- CSV export of computed spectrum (`--out`)
- DFT vs FFT runtime benchmark tool (`fft_benchmark`)

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
bench/                performance benchmarks
.github/              CI workflows
```

---

## Requirements

- CMake 3.25+
- Ninja
- C++20 compatible compiler
- Linux environment is recommended

---

## Development

Development notes and CI instructions are available in [docs/DEVELOPMENT.md](docs/DEVELOPMENT.md).

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
X[k] = Σ x[n] · e^(−j2πkn/N),  k = 0..N−1
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
- Additional window functions
- Spectrogram computation

---

## License

MIT License
