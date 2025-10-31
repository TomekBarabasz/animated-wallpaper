#include <matrix2.hpp>
#include <vector>
#include "XoshiroCpp.hpp"
#include <random>
#include <algorithm>
#include <cstdint>  // For uint32_t

namespace {
uint64_t gen_seed() 
{
#if defined(__x86_64__) || defined(_M_X64)
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

template <typename T>
Matrix<T> Matrix<T>::randu(size_t r, size_t c, size_t seed)
{
    Matrix<T> m(r, c);
    using namespace XoshiroCpp;
    
    XoshiroRNG rng(seed != 0 ? seed : gen_seed());

    auto gen_float = [&]() -> float {
        uint32_t u = rng() >> 8;  // 24-bit mantissa for float [0,1)
        return (u + 0.5f) * (1.0f / (1u << 24));
    };
    std::generate(m.data, m.data + r * c, gen_float);
    
    return m;
}

template <typename T>
Matrix<T> Matrix<T>::randn(size_t r, size_t c, size_t seed)
{
    Matrix<T> m(r, c);

    XoshiroRNG rng(seed != 0 ? seed : gen_seed());

    std::normal_distribution<T> dist(0, 1);
    std::generate(m.data, m.data + r * c, [&]() { return dist(rng); });
    
    return m;
}
