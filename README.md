# animated-wallpaper
animated wallpaper in Rust, Julia and C++
for performance and language caps comparison
using Gray Scott reaction-diffusion PDE

# how to run julia
TBD

# how to build with msvc
- Open a Developer Command Prompt i.e x64 Native Tools Command Prompt for VS 2022
- `cmake -G "Visual Studio 17 2022" -A x64 -DCMAKE_BUILD_TYPE=Release ..`
` `cmake --build . --config Release`

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

## "Visual Studio 17 2022"
compiled with /O2 /AVX2
`C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.44.35207\bin\HostX64\x64\CL.exe /c /I"C:\tom
  ek\projects\animated-wallpaper\c++\include" /I"C:\tomek\projects\animated-wallpaper\c++\build\_deps\xoshiro-src" /nol
  ogo /W3 /WX- /diagnostics:column /O2 /Ob2 /D _MBCS /D WIN32 /D _WINDOWS /D NDEBUG /D "CMAKE_INTDIR=\"Release\"" /EHsc
   /MD /GS /arch:AVX2 /fp:fast /GR /std:c++20 /Fo"gray-scott-lib.dir\Release\\" /Fd"C:\tomek\projects\animated-wallpape
  r\c++\build\Release\gray-scott-lib.pdb" /external:W3 /Gd /TP /errorReport:queue "C:\tomek\projects\animated-wallpaper
  \c++\src\conv2d.cpp"`

| Benchmark                 | Time (ns) | CPU (ns) | Iterations |
|----------------------------|-----------|-----------|-------------|
| BM_conv3x3_f32/128         | 64,686    | 64,174    | 11,200      |
| BM_conv3x3_f32/256         | 261,518   | 260,911   | 2,635       |
| BM_conv3x3_f32/512         | 1,056,452 | 1,045,850 | 747         |
| BM_conv3x3_f32_avx2/128    | 9,531     | 9,305     | 154,483     |
| BM_conv3x3_f32_avx2/256    | 37,220    | 35,157    | 21,333      |
| BM_conv3x3_f32_avx2/512    | 140,617   | 131,830   | 4,978       |

## rust
TBD

## julia
TBD
