#include <benchmark/benchmark.h>
#include <matrix.hpp>
#include <matrix_ops.hpp>
#include <cstring>

using namespace matrix;
using namespace matrix::ops;

float kernel_data[] = {
    0.05f, .20f, 0.05f,
    .02f, -1.0f, .2f,
    0.05f, .2f, 0.05f
};

static void BM_conv3x3_f32(benchmark::State& state) {
    const size_t n = state.range(0);
    auto A = randu<float>(n,n);
    auto B1 = zeros<float>(A.get_shape());
    auto B2 = zeros<float>(n,n);
    auto K = zeros<float>(3,3);
    std::memcpy(K.get_data(), kernel_data, sizeof(kernel_data));

    for (auto _ : state) {
         conv3x3_f32(A, K, B1);
    }
}
static void BM_conv3x3_f32_avx2(benchmark::State& state) {
    const size_t n = state.range(0);
    auto A = randu<float>(n,n);
    auto B1 = zeros<float>(A.get_shape());
    auto B2 = zeros<float>(n,n);
    auto K = zeros<float>(3,3);
    std::memcpy(K.get_data(), kernel_data, sizeof(kernel_data));

    for (auto _ : state) {
         conv3x3_f32_avx2(A, K, B1);
    }
}

BENCHMARK(BM_conv3x3_f32)->Arg(128)->Arg(256)->Arg(512);
BENCHMARK(BM_conv3x3_f32_avx2)->Arg(128)->Arg(256)->Arg(512);

BENCHMARK_MAIN();
