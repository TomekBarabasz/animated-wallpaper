# animated-wallpaper
animated wallpaper in Rust, Julia and C++
for performance and language caps comparison

# perf results

## clang-20
| Benchmark                 | Time (ns) | CPU (ns) | Iterations |
|----------------------------|-----------|-----------|-------------|
| BM_conv3x3_f32/128         | 5967      | 5966      | 121102      |
| BM_conv3x3_f32/256         | 20611     | 20609     | 34540       |
| BM_conv3x3_f32/512         | 89245     | 89241     | 8736        |
| BM_conv3x3_f32_avx2/128    | 5859      | 5859      | 93367       |
| BM_conv3x3_f32_avx2/256    | 21210     | 21209     | 33057       |
| BM_conv3x3_f32_avx2/512    | 90804     | 90800     | 6698        |

## gcc-13
| Benchmark                 | Time (ns) | CPU (ns) | Iterations |
|----------------------------|-----------|-----------|-------------|
| BM_conv3x3_f32/128         | 6782      | 6782      | 108297      |
| BM_conv3x3_f32/256         | 25183     | 25182     | 27979       |
| BM_conv3x3_f32/512         | 91517     | 91515     | 8188        |
| BM_conv3x3_f32_avx2/128    | 6297      | 6297      | 116067      |
| BM_conv3x3_f32_avx2/256    | 21656     | 21655     | 30843       |
| BM_conv3x3_f32_avx2/512    | 84247     | 84245     | 8251        |

## rust
TBD

## julia
TBD
