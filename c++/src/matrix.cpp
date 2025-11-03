#include <matrix.hpp>
#include <vector>
#include "XoshiroCpp.hpp"
#include <random>
#include <algorithm>
#include <cstdint>  // For uint32_t

#ifdef MSVC_VER
#include <immintrin.h>
#endif

namespace {
uint64_t gen_seed() 
{
#ifdef _MSC_VER
    return __rdtsc();
#elif defined(__x86_64__) || defined(_M_X64)
    unsigned lo, hi;
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
#elif defined(__aarch64__)
    uint64_t val;
    asm volatile("mrs %0, cntvct_el0" : "=r"(val));
    return val;
#else
    // fallback
    return std::chrono::steady_clock::now().time_since_epoch().count();
#endif
}
}

using XoshiroRNG = XoshiroCpp::Xoshiro256PlusPlus;

namespace matrix
{
template <>
void randu<float>(float* data, size_t size)
{
    XoshiroRNG rng(gen_seed());
    auto gen_float = [&]() -> float {
        uint32_t u = rng() >> 8;  // 24-bit mantissa for float [0,1)
        return (u + 0.5f) * (1.0f / (1u << 24));
    };
    std::generate(data, data + size, gen_float);
}

template <>
void randn<float>(float* data, size_t size)
{
    XoshiroRNG rng(gen_seed());
    std::normal_distribution<float> dist(0, 1);
    std::generate(data, data + size, [&]() { return dist(rng); });
}
}