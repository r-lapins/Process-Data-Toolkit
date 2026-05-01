cmake_minimum_required(VERSION 3.25)
project(process_data_toolkit VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(PDT_BUILD_CSV "Build CSV CLI" ON)
option(PDT_BUILD_WAV "Build WAV CLI and demos" ON)
option(PDT_BUILD_TESTS "Build tests" ON)
option(PDT_BUILD_BENCH "Build benchmarks" ON)
option(PDT_ENABLE_SANITIZERS "Enable sanitizers" ON)
option(PDT_ENABLE_CUDA "Enable CUDA FFT backend" OFF)
option(PDT_BUILD_RTLSDR "Build RTL-SDR tools" OFF)

# Sanitizers (ASAN/UBSAN)
if(PDT_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_library(pdt_sanitizers INTERFACE)
    target_compile_options(pdt_sanitizers INTERFACE -fsanitize=address,undefined)
    target_link_options(pdt_sanitizers INTERFACE -fsanitize=address,undefined)
endif()

# CSV libraries
add_library(pdt_csv
    src/csv/csv_reader.cpp
    src/csv/csv_output.cpp
    src/csv/time.cpp
    src/csv/dataset.cpp
    src/csv/anomaly.cpp
)
target_include_directories(pdt_csv PUBLIC include)

# DSP libraries
add_library(pdt_dsp
    src/dsp/complex_fft.cpp
    src/dsp/dft.cpp
    src/dsp/fft.cpp
    src/dsp/peak_detection.cpp
    src/dsp/window.cpp
    src/compute/cpu_fft_backend.cpp
    src/compute/make_fft_backend.cpp
    src/compute/fft_size_policy.cpp
    src/pipeline/spectrum_engine.cpp
    src/pipeline/wav_analysis_service.cpp
    src/pipeline/wav_analysis_session.cpp
)
target_include_directories(pdt_dsp PUBLIC include)

# CUDA
if(PDT_ENABLE_CUDA)
    find_package(CUDAToolkit REQUIRED)
    target_sources(pdt_dsp PRIVATE src/compute/cuda_fft_backend.cpp)
    target_link_libraries(pdt_dsp PRIVATE CUDA::cudart CUDA::cufft)
    target_compile_definitions(pdt_dsp PUBLIC PDT_ENABLE_CUDA)
endif()

# I/O WAV
add_library(pdt_io_wav
    src/io/wav/wav_output.cpp
    src/io/wav/wav_reader.cpp
)
target_include_directories(pdt_io_wav PUBLIC include)
target_link_libraries(pdt_io_wav PUBLIC pdt_dsp)

# I/O RTL-SDR
if(PDT_BUILD_RTLSDR)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(RTLSDR REQUIRED IMPORTED_TARGET librtlsdr)

    add_library(pdt_io_rtlsdr
        src/io/rtlsdr/rtl_sdr_device.cpp
        src/io/rtlsdr/rtl_sdr_stream.cpp
        src/io/rtlsdr/iq_ring_buffer.cpp
        src/pipeline/iq_spectrum_engine.cpp
    )

    target_include_directories(pdt_io_rtlsdr PUBLIC include)
    target_link_libraries(pdt_io_rtlsdr PUBLIC PkgConfig::RTLSDR)
endif()

# Sanitizers
if(TARGET pdt_sanitizers)
    target_link_libraries(pdt_csv PUBLIC pdt_sanitizers)
    target_link_libraries(pdt_dsp PUBLIC pdt_sanitizers)
    target_link_libraries(pdt_io_wav PUBLIC pdt_sanitizers)

    if(TARGET pdt_io_rtlsdr)
        target_link_libraries(pdt_io_rtlsdr PUBLIC pdt_sanitizers)
    endif()
endif()

# CLI helpers
add_library(pdt_common_cli STATIC app/common_cli.cpp)
target_include_directories(pdt_common_cli PUBLIC app)

# CSV CLI
if(PDT_BUILD_CSV)
    add_library(pdt_csv_cli_args STATIC app/csv_cli_args.cpp)
    target_include_directories(pdt_csv_cli_args PUBLIC app)
    target_link_libraries(pdt_csv_cli_args PUBLIC pdt_common_cli pdt_csv)

    add_executable(pdt_csv_cli app/csv_main.cpp)
    target_link_libraries(pdt_csv_cli PRIVATE pdt_csv_cli_args)
endif()

# DSP-WAV CLI
if(PDT_BUILD_WAV)
    add_library(pdt_wav_cli_args STATIC app/wav_cli_args.cpp)
    target_include_directories(pdt_wav_cli_args PUBLIC app)
    target_link_libraries(pdt_wav_cli_args PUBLIC pdt_common_cli pdt_dsp)

    add_executable(pdt_wav_cli app/wav_main.cpp)
    target_link_libraries(pdt_wav_cli PRIVATE pdt_wav_cli_args pdt_io_wav)

    add_executable(pdt_wav_synth_demo app/wav_synth_demo.cpp)
    target_link_libraries(pdt_wav_synth_demo PRIVATE pdt_common_cli pdt_dsp)
endif()

# Tests
if(PDT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Benchmarks
if(PDT_BUILD_BENCH AND PDT_ENABLE_CUDA)
    add_executable(fft_benchmark bench/fft_benchmark.cpp)
    target_link_libraries(fft_benchmark PRIVATE pdt_dsp)

    add_executable(fft_benchmark_wav bench/fft_benchmark_wav.cpp)
    target_link_libraries(fft_benchmark_wav PRIVATE pdt_dsp pdt_io_wav)
endif()

if(PDT_BUILD_RTLSDR)
    add_executable(rtl_sdr_smoke app/rtl_sdr_smoke.cpp)
    target_link_libraries(rtl_sdr_smoke PRIVATE pdt_io_rtlsdr)
endif()
