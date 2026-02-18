# Process Data Toolkit

Modern C++20 **library + CLI** for processing industrial sensor CSV data.

Built like a production-style project: CMake presets, unit tests, sanitizers, CI.

## Build

```bash
cmake --preset debug
cmake --build --preset debug
ctest --preset debug
```

## Run

```bash
./build/debug/pdt_cli
```

## Features (MVP)

- CSV parsing (`timestamp,sensor,value`)
- Basic statistics: `count`, `mean`, `min`, `max`, `stddev`
- Library + CLI separation
- CMake presets (Debug/Release)
- ASAN + UBSAN in Debug
- GitHub Actions CI
- clang-format / clang-tidy configs

## Project structure

```
include/pdt/   public API
src/           library implementation
app/           CLI
tests/         unit tests
.github/       CI pipeline
```

## Roadmap

- CLI args: `--in`, `--sensor`, `--out`
- Timestamp parsing (ISO 8601 → std::chrono)
- JSON/CSV report export
- Simple anomaly detection
