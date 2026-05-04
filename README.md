# Process Data Toolkit (PDT)

![CI](https://github.com/r-lapins/Process-Data-Toolkit/actions/workflows/ci.yml/badge.svg)
![CUDA optional](https://img.shields.io/badge/CUDA-optional-green)

Modern C++20 library and CLI tools for CSV time-series processing, WAV signal analysis, and optional RTL-SDR live spectrum input.

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

- CSV parsing, filtering and statistics
- Per-sensor analysis
- Configurable anomaly detection (`zscore`, `iqr`, `mad`)
- JSON and marked-CSV export with anomaly highlighting

### WAV signal analysis

[Notes and instructions are available here: docs/WAV.md](docs/WAV.md).

- WAV spectrum analysis CLI (`pdt_wav_cli`)
- CPU/CUDA FFT backend abstraction
- Windowing, peak detection and dominant frequency reporting
- CSV/text report export
- Synthetic signal demo and FFT benchmarks

### RTL-SDR live spectrum input

[Notes and instructions are available here: docs/RTLSDR.md](docs/RTLSDR.md).

- Optional live SDR input module (`PDT_BUILD_RTLSDR=ON`)
- RTL-SDR device setup and asynchronous IQ streaming
- IQ buffering and live spectrum analysis
- CPU/CUDA FFT backend support
- Hardware smoke-test executable (`rtl_sdr_smoke`)

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
- `debug-rtlsdr-cuda-nosan` — RTL-SDR tools enabled, CUDA enabled, no sanitizers
- `release` — optimized CPU build
- `release-cuda` — optimized CUDA build

Note: CUDA builds require sanitizers to be disabled.

---

## Architecture

The project is organized in two complementary views:

### Domain modules

- `csv` — CSV time-series processing, filtering, statistics, anomaly detection
- `wav` — offline signal/spectrum analysis for WAV input
- `rtlsdr` — optional live RTL-SDR IQ input

### Core layers

- `pdt/io` — input/output modules (WAV, RTL-SDR)
- `pdt/dsp` — DFT, FFT, windows, peak detection
- `pdt/compute` — CPU/CUDA FFT backends
- `pdt/pipeline` — analysis orchestration

---

## Project structure

```md
include/pdt/   Public C++ API
src/           Library implementations
app/           CLI tools and smoke-test executables
tests/         Unit and smoke tests
bench/         FFT and pipeline benchmarks
docs/          Module documentation
examples/      Sample inputs and generated outputs
.github/       CI workflows
```

---

## Requirements

- CMake
- Ninja
- C++20 compiler
- optional CUDA
- optional RTL-SDR

### Optional CUDA support

CUDA backend can be enabled to accelerate FFT computation. 

Requirements:

- NVIDIA GPU
- CUDA Toolkit (with cuFFT)

When enabled:
- `CudaFftBackend` and `fft_benchmark` are available
- FFT can be executed on GPU (cuFFT)

### Optional RTL-SDR support

RTL-SDR tools require `librtlsdr` and `pkg-config`.
See [docs/RTLSDR.md](docs/RTLSDR.md).

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
