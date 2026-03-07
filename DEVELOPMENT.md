# Development notes

## Running clang-tidy locally

Clean build directory:

rm -rf build/debug

Configure with clang and clang-tidy enabled:

CC=clang CXX=clang++ \
cmake --preset debug \
-DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
-DCMAKE_CXX_CLANG_TIDY=clang-tidy

Build project (clang-tidy will run automatically):

cmake --build --preset debug


## Requirements

Fedora:

sudo dnf install clang clang-tools-extra ninja-build cmake

Ubuntu:

sudo apt-get install clang clang-tidy ninja-build cmake


## CI

CI pipeline runs three jobs:

1. GCC build + tests
2. Clang build + sanitizers
3. clang-tidy static analysis

Workflow file:

.github/workflows/ci.yml