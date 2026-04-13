# Process Data Toolkit (PDT)

![CI](https://github.com/r-lapins/Process-Data-Toolkit/actions/workflows/ci.yml/badge.svg)
![CUDA optional](https://img.shields.io/badge/CUDA-optional-green)

Modern C++20 library and CLI tools for CSV time-series processing and WAV signal analysis.

---

## Related project

This library powers a desktop application:

👉 [Process Data Viewer (Qt)](https://github.com/r-lapins/Process-Data-Viewer-Qt)

The viewer provides an interactive GUI for:
- CSV anomaly analysis
- WAV signal and spectrum analysis
- visualization and export tools

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

## Development

[Development notes and CI instructions are available here: docs/DEVELOPMENT.md](docs/DEVELOPMENT.md).

---

## Features

### CSV data processing

[Notes and instructions are available here: docs/CSV.md](docs/CSV.md).

- CLI data processing tool for CSV files (`pdt_csv_cli`)
- CSV parser with import summary (`parsed_ok`, `skipped`)
- Optional display of skipped CSV rows with line numbers (`--skipped`)
- ISO 8601 timestamp parsing using `std::chrono`
- Data filtering by sensor and time range
- Domain model based on `DataSet` class
- Statistical analysis (`count`, `mean`, `min`, `max`, `stddev`)
- Per-sensor statistics mode (`--per-sensor`)
- Configurable anomaly detection (`zscore`, `iqr`, `mad`) with threshold and top-N output (`--z`, `--method`, `--top`)
- JSON report export (`--out`)
- CSV export with anomaly markers for top detected anomalies (`--out-marked-csv`)

### WAV signal analysis

[Notes and instructions are available here: docs/WAV.md](docs/WAV.md).

- CLI spectrum analysis tool for WAV files (`pdt_wav_cli`)
- Discrete Fourier Transform (DFT)
- Radix-2 Fast Fourier Transform (FFT)
- Backend abstraction for spectrum computation (CPU / CUDA)
- Automatic DFT / FFT selection (CPU backend)
- Optional CUDA acceleration using **cuFFT**
- Single-sided spectrum computation
- Window functions: Hann and Hamming
- Spectral peak detection (`ThresholdOnly`, `LocalMaxima`)
- Detection of peaks and selection of the dominant peak separately
- WAV reader (RIFF/WAVE PCM16 mono)
- Synthetic signal spectrum analysis demo (`pdt_wav_synth_demo`)
- CSV export of computed spectrum (`--out`)
- Text report export (`--out-r`)
- FFT benchmark tool (DFT vs CPU FFT vs CUDA FFT - `fft_benchmark`)

#### Example outputs

CSV CLI can:
- print import/skipped row summaries
- generate JSON reports
- export anomaly-marked CSV files

WAV CLI can:
- print spectral peak reports
- export spectrum CSV
- export text reports

---

## Quick start

```bash
cmake --preset debug
cmake --build --preset debug
```

Run CSV CLI:

```bash
./build/debug/pdt_csv_cli --in examples/sample.csv
```

Run WAV CLI:

```bash
./build/debug/pdt_wav_cli --in examples/HDSDR_20230515_072359Z_15047kHz_AF.wav
```

---

## CMake presets

- `debug` — CPU only (default, sanitizers enabled)
- `debug-nosan` — CPU only, no sanitizers
- `debug-cuda-nosan` — CUDA enabled, no sanitizers
- `release` — optimized CPU build
- `release-cuda` — optimized CUDA build

Note: CUDA builds require sanitizers to be disabled.

---

## Architecture

The project is organized in two complementary views:

### Domain modules

- `csv` — CSV time-series processing, filtering, statistics, anomaly detection
- `wav` — offline signal/spectrum analysis for WAV input
- `rtlsdr` — planned live SDR input module

### Core layers

- `pdt/io` — input/output modules (currently WAV I/O, later also RTL-SDR)
- `pdt/dsp` — core DSP algorithms: DFT, FFT, windows, peak detection, spectrum types
- `pdt/compute` — spectrum computation backends (`IFftBackend`, CPU, CUDA/cuFFT)
- `pdt/pipeline` — backend-driven analysis flow (`SpectrumEngine`)

---

## Project structure

```md
include/pdt/
├── compute/        FFT/spectrum backends (CPU, CUDA)
├── csv/            CSV processing public API
├── dsp/            DSP public API
├── io/
│   └── wav/        WAV I/O public API
└── pipeline/       analysis pipeline public API

src/
├── compute/        backend implementations
├── csv/            CSV processing implementation
├── dsp/            DSP implementation
├── io/
│   └── wav/        WAV I/O implementation
└── pipeline/       analysis pipeline implementation

app/                CLI applications
bench/              performance benchmarks
tests/              unit tests
examples/           sample CSV and WAV inputs and outputs
.github/            CI workflows
```

---

## Requirements

- CMake 3.25+
- Ninja
- C++20 compatible compiler
- Linux environment is recommended

### Optional CUDA support

CUDA backend can be enabled to accelerate FFT computation. 

Requirements:

- NVIDIA GPU
- CUDA Toolkit (with cuFFT)

When enabled:
- `CudaFftBackend` and `fft_benchmark` are available
- FFT can be executed on GPU (cuFFT)

---

## Algorithms

#### Standard deviation:

```md
σ = sqrt( Σ(x - μ)² / N )
```

### Anomaly detection methods:

The CSV CLI supports three anomaly detection methods:

####  - Z-score

```md
z = (x - μ) / σ
```

Samples with `|z| > threshold` are reported as anomalies.

####  - IQR

The interquartile range method uses:

```md
IQR = Q3 - Q1
```

Samples outside the interval

[Q1 - threshold · IQR, Q3 + threshold · IQR]

are reported as anomalies.

####  - MAD

The median absolute deviation method uses:

```md
MAD = median(|x - median(x)|)
```

A robust anomaly score is computed:

```md
score = (x - median(x)) / MAD
```

Samples with `|score| > threshold` are reported as anomalies.

### WAV signal processing methods:

#### - Discrete Fourier Transform (DFT)

```md
X[k] = Σ x[n] · e^(−j2πkn/N),  k = 0..N−1
```

Current implementation is `O(N²)` and serves as a reference implementation.

#### - Fast Fourier Transform (FFT)

The project implements a radix-2 Cooley–Tukey FFT algorithm.

The FFT recursively decomposes the DFT into even and odd indexed samples:

```md
X[k] = E[k] + W_N^k · O[k]
X[k + N/2] = E[k] - W_N^k · O[k]
```

where:

```md
W_N^k = e^(−j2πk/N)
```

The algorithm requires the input size to be a power of two and has time complexity `O(N log N)`

#### - Spectral peak detection

Two strategies:

**ThresholdOnly**

```md
X[i] >= threshold_ratio · max(X)
```

**LocalMaxima**

```md
X[i] > X[i-1] && X[i] > X[i+1]
```

---

## Library usage

Example:

```cpp
#include <pdt/csv/dataset.h>
#include <pdt/csv/csv_reader.h>
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

- Streaming / online anomaly detection
- Additional window functions
- Spectrogram computation

---

## License

MIT License
