cmake_minimum_required(VERSION 3.12)
project(gray-scott VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
  benchmark
  GIT_REPOSITORY https://github.com/google/benchmark.git
  GIT_TAG v1.8.3  # Check latest version at https://github.com/google/benchmark/releases
)

FetchContent_Declare(
  xoshiro
  GIT_REPOSITORY https://github.com/Reputeless/Xoshiro-cpp.git
  GIT_TAG v1.1  # Check latest version at https://github.com/Reputeless/Xoshiro-cpp/releases
)

FetchContent_Declare(
  eigen
  GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
  GIT_TAG 5.0.0  # Check latest version at https://gitlab.com/libeigen/eigen/-/tags
)


# Also fetch GoogleTest, which Benchmark uses for testing (optional)
set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "Disable tests" FORCE)
set(BENCHMARK_DOWNLOAD_DEPENDENCIES ON CACHE BOOL "Download dependencies" FORCE)

FetchContent_MakeAvailable(benchmark)
FetchContent_MakeAvailable(xoshiro)
FetchContent_MakeAvailable(eigen)

add_library(gray-scott-lib STATIC
    src/gray_scott.cpp
    src/matrix.cpp
    src/conv2d.cpp
)
target_include_directories(gray-scott-lib PRIVATE 
    include
    ${xoshiro_SOURCE_DIR}
)

add_executable(gray-scott
    src/main.cpp
)
target_include_directories(gray-scott PRIVATE include)
target_link_libraries(gray-scott PRIVATE gray-scott-lib eigen)

add_executable(gs_benchmark
    src/benchmark.cpp
)
target_include_directories(gs_benchmark PRIVATE include)
target_link_libraries(gs_benchmark PRIVATE gray-scott-lib benchmark::benchmark)


if (MSVC)
    set(compile_options
        $<$<CONFIG:Release>:/O2 /fp:fast /arch:AVX2 /DNDEBUG>
        $<$<CONFIG:Debug>:/Od /Zi /W4 /arch:AVX2>
    )
else()
    set(compile_options
        $<$<CONFIG:Release>:-O3 -march=native -ffast-math -mavx2 -mfma -DNDEBUG>
        $<$<CONFIG:Debug>:-g -O0 -Wall -Wextra -Wpedantic -mavx2 -mfma>
    )
endif()

target_compile_options(gray-scott-lib PRIVATE ${compile_options})
target_compile_options(gray-scott PRIVATE ${compile_options} )
target_compile_options(gs_benchmark PRIVATE ${compile_options} )
