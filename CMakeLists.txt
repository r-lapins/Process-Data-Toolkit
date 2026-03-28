cmake_minimum_required(VERSION 3.25)
project(process_data_toolkit VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PDT_ENABLE_SANITIZERS "Enable sanitizers" ON)

option(PDT_BUILD_CSV "Build CLI apps" ON)
option(PDT_BUILD_WAV "Build demos" ON)
option(PDT_BUILD_TESTS "Build tests" ON)
option(PDT_BUILD_BENCH "Build benchmarks" ON)

# Sanitizers (ASAN/UBSAN) – apply to *all* targets that link with pdt
if(PDT_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_library(pdt_sanitizers INTERFACE)
    target_compile_options(pdt_sanitizers INTERFACE -fsanitize=address,undefined)
    target_link_options(pdt_sanitizers INTERFACE -fsanitize=address,undefined)
endif()

add_library(pdt
    src/csv/csv_reader.cpp
    src/csv/time.cpp
    src/csv/dataset.cpp
    src/csv/output.cpp
    src/csv/anomaly.cpp

    src/wav/dft.cpp
    src/wav/fft.cpp
    src/wav/peak_detection.cpp
    src/wav/spectrum_output.cpp
    src/wav/wav_reader.cpp
    src/wav/window.cpp
)
target_include_directories(pdt PUBLIC include)

if(TARGET pdt_sanitizers)
    target_link_libraries(pdt PUBLIC pdt_sanitizers)
endif()

add_library(pdt_common_cli STATIC app/common_cli.cpp)
target_include_directories(pdt_common_cli PUBLIC app)
target_link_libraries(pdt_common_cli PUBLIC pdt)

if(PDT_BUILD_CSV)
    add_library(pdt_csv_cli_args STATIC app/csv_cli_args.cpp)
    target_include_directories(pdt_csv_cli_args PUBLIC app)
    target_link_libraries(pdt_csv_cli_args PUBLIC pdt_common_cli)

    add_executable(pdt_csv_cli app/csv_main.cpp)
    target_link_libraries(pdt_csv_cli PRIVATE pdt_csv_cli_args)
endif()

if(PDT_BUILD_WAV)
    add_library(pdt_wav_cli_args STATIC app/wav_cli_args.cpp)
    target_include_directories(pdt_wav_cli_args PUBLIC app)
    target_link_libraries(pdt_wav_cli_args PUBLIC pdt_common_cli)

    add_executable(pdt_wav_cli app/wav_main.cpp)
    target_link_libraries(pdt_wav_cli PRIVATE pdt_wav_cli_args)

    add_executable(pdt_wav_synth_demo app/wav_synth_demo.cpp)
    target_link_libraries(pdt_wav_synth_demo PRIVATE pdt_common_cli)
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
