#include <benchmark/benchmark.h>
#include <vector>
#include <algorithm>
#include <numeric>
#include <random>

static void BM_SortVector(benchmark::State& state) {
    for (auto _ : state) {
        std::vector<int> v(state.range(0));
        std::iota(v.begin(), v.end(), 0);
        std::shuffle(v.begin(), v.end(), std::mt19937{std::random_device{}()});
        std::sort(v.begin(), v.end());
    }
}

BENCHMARK(BM_SortVector)->Range(8, 8 << 10);

BENCHMARK_MAIN();
