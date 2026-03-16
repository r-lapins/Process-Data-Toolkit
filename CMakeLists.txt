cmake_minimum_required(VERSION 3.25)
project(process_data_toolkit VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PDT_ENABLE_SANITIZERS "Enable sanitizers" ON)

option(PDT_BUILD_TESTS "Build tests" ON)
option(PDT_BUILD_CLI "Build CLI apps" ON)
option(PDT_BUILD_SPECTRUM "Build demos" ON)
option(PDT_BUILD_BENCH "Build benchmarks" ON)

# Sanitizers (ASAN/UBSAN) – apply to *all* targets that link with pdt
if(PDT_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_library(pdt_sanitizers INTERFACE)
    target_compile_options(pdt_sanitizers INTERFACE -fsanitize=address,undefined)
    target_link_options(pdt_sanitizers INTERFACE -fsanitize=address,undefined)
endif()

add_library(pdt
    src/core/csv_reader.cpp
    src/core/time.cpp
    src/core/dataset.cpp
    src/core/report.cpp
    src/core/anomaly.cpp
    src/signal/dft.cpp
    src/signal/peak_detection.cpp
    src/signal/wav_reader.cpp
    src/signal/fft.cpp
    src/signal/window.cpp
    src/signal/csv_spectrum_export.cpp
)
target_include_directories(pdt PUBLIC include)

if(TARGET pdt_sanitizers)
    target_link_libraries(pdt PUBLIC pdt_sanitizers)
endif()

add_library(pdt_cli_common STATIC app/cli_common.cpp)
target_include_directories(pdt_cli_common PUBLIC app)
target_link_libraries(pdt_cli_common PUBLIC pdt)

if(PDT_BUILD_CLI)
    add_library(pdt_cli_args STATIC app/cli_args.cpp)
    target_include_directories(pdt_cli_args PUBLIC app)
    target_link_libraries(pdt_cli_args PUBLIC pdt_cli_common)

    add_executable(pdt_cli app/main.cpp)
    target_link_libraries(pdt_cli PRIVATE pdt_cli_args)
endif()

if(PDT_BUILD_SPECTRUM)
    add_library(pdt_spectrum_cli_args STATIC app/spectrum_cli_args.cpp)
    target_include_directories(pdt_spectrum_cli_args PUBLIC app)
    target_link_libraries(pdt_spectrum_cli_args PUBLIC pdt_cli_common)

    add_executable(spectrum_cli app/spectrum_main.cpp)
    target_link_libraries(spectrum_cli PRIVATE pdt_spectrum_cli_args)

    add_executable(spectrum_synth_demo app/spectrum_synth_demo.cpp)
    target_link_libraries(spectrum_synth_demo PRIVATE pdt_cli_common)
endif()

if(PDT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(PDT_BUILD_BENCH)
    add_executable(fft_benchmark bench/fft_benchmark.cpp)
    target_link_libraries(fft_benchmark PRIVATE pdt)
endif()

# include(GNUInstallDirs)
# install(TARGETS process_data_toolkit
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
# )
