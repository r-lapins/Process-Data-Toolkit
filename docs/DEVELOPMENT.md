# Development notes

## Requirements

#### Fedora:

```
sudo dnf install clang clang-tools-extra ninja-build cmake
```

#### Ubuntu:

```
sudo apt-get install clang clang-tidy ninja-build cmake
```

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

Without sanitizers:

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

```bash
valgrind --leak-check=full --track-origins=yes \
./build/debug-nosan/spectrum_cli \
--in examples/HDSDR_20230515_072359Z_15047kHz_AF.wav \
--threshold 0.01 --window hamming --from 4096 --window-size 2048
```

---

## clang-tidy

Clean build directory:

```
rm -rf build/debug
```

Configure with clang and clang-tidy enabled:

```
cmake --preset debug \
-DCMAKE_C_COMPILER=clang \
-DCMAKE_CXX_COMPILER=clang++ \
-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
-DCMAKE_CXX_CLANG_TIDY=clang-tidy
```

Build project (clang-tidy will run automatically):

```
cmake --build --preset debug
```

## Formatting

clang-format is used for consistent code style.

Run:

```
clang-format -i $(git ls-files '*.cpp' '*.h')
```

## CI

The CI pipeline runs three jobs:

1. GCC build + tests
2. Clang build + sanitizers
3. clang-tidy static analysis

Workflow file:

```
.github/workflows/ci.yml
```
