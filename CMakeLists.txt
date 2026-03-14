cmake_minimum_required(VERSION 3.25)
project(process_data_toolkit VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PDT_ENABLE_SANITIZERS "Enable sanitizers" ON)

option(PDT_BUILD_TESTS "Build tests" ON)
option(PDT_BUILD_CLI "Build CLI apps" ON)
option(PDT_BUILD_SPECTRUM "Build demos" ON)

# Sanitizers (ASAN/UBSAN) – apply to *all* targets that link with pdt
if(PDT_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_library(pdt_sanitizers INTERFACE)
    target_compile_options(pdt_sanitizers INTERFACE -fsanitize=address,undefined)
    target_link_options(pdt_sanitizers INTERFACE -fsanitize=address,undefined)
endif()

add_library(pdt
    src/csv_reader.cpp
    src/time.cpp
    src/dataset.cpp
    src/report.cpp
    src/anomaly.cpp
    src/dft.cpp
    src/peak_detection.cpp
    src/wav_reader.cpp
    src/fft.cpp
    src/window.cpp
)
target_include_directories(pdt PUBLIC include)

if(TARGET pdt_sanitizers)
    target_link_libraries(pdt PUBLIC pdt_sanitizers)
endif()

if(PDT_BUILD_CLI)
    add_library(pdt_cli_args app/cli_args.cpp)
    target_include_directories(pdt_cli_args PUBLIC app)
    target_link_libraries(pdt_cli_args PUBLIC pdt)

    add_executable(pdt_cli app/main.cpp)
    target_link_libraries(pdt_cli PRIVATE pdt_cli_args)
endif()

if(PDT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(PDT_BUILD_SPECTRUM)
    add_library(pdt_spectrum_cli_args STATIC app/spectrum_cli_args.cpp)
    target_include_directories(pdt_spectrum_cli_args PUBLIC app)
    target_link_libraries(pdt_spectrum_cli_args PUBLIC pdt)

    add_executable(spectrum_cli app/spectrum_main.cpp)
    target_link_libraries(spectrum_cli PRIVATE pdt pdt_spectrum_cli_args)

    add_executable(spectrum_synth_demo app/spectrum_synth_demo.cpp)
    target_link_libraries(spectrum_synth_demo PRIVATE pdt)
endif()

# include(GNUInstallDirs)
# install(TARGETS process_data_toolkit
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
# )
