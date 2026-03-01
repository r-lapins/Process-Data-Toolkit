cmake_minimum_required(VERSION 3.25)
project(process_data_toolkit VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(PDT_ENABLE_SANITIZERS "Enable sanitizers" ON)

# Sanitizers (ASAN/UBSAN) – apply to *all* targets that link with pdt
if(PDT_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_library(pdt_sanitizers INTERFACE)
    target_compile_options(pdt_sanitizers INTERFACE -fsanitize=address,undefined)
    target_link_options(pdt_sanitizers INTERFACE -fsanitize=address,undefined)
endif()

add_library(pdt
    src/csv_reader.cpp
    src/stats.cpp
    src/time.cpp
    src/filter.cpp
)

target_include_directories(pdt PUBLIC include)

if(TARGET pdt_sanitizers)
    target_link_libraries(pdt PUBLIC pdt_sanitizers)
endif()

add_executable(pdt_cli app/main.cpp)
target_link_libraries(pdt_cli PRIVATE pdt)

enable_testing()
add_subdirectory(tests)

# include(GNUInstallDirs)
# install(TARGETS process_data_toolkit
#     LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#     RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
# )
