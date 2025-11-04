#include <matrix.hpp>
#include <immintrin.h>

using matrix::Matrix;
namespace matrix::ops {
void conv3x3_f32(const Matrix<float,2>& input, const Matrix<float,2>& kernel, Matrix<float,2>& output)
{
    const auto n_rows = input.get_shape()[0];
    const auto n_cols = input.get_shape()[1];

    auto inp = input.view();
    auto outp = output.view();
    auto kern = kernel.view();

    for (unsigned i = 1; i < n_rows - 1; ++i) {
        for (unsigned j = 1; j < n_cols - 1; ++j) {
            float sum = 0.0f;
            for (int ki = -1; ki <= 1; ++ki) {
                for (int kj = -1; kj <= 1; ++kj) {
                    sum += inp[i + ki][j + kj] * kern[ki + 1][kj + 1];
                }
            }
            outp[i][j] = sum;
        }
    }
}

void conv3x3_f32_avx2(const Matrix<float,2>& input, const Matrix<float,2>& kernel, Matrix<float,2>& output) 
{
    const auto n_rows = input.get_shape()[0];
    const auto n_cols = input.get_shape()[1];

    auto inp = input.view();
    auto outp = output.view();

    const float* __restrict src = input.get_data();
    const float* __restrict kern = kernel.get_data();
    float* __restrict dst = output.get_data();
    int width = input.get_shape()[1];
    int height = input.get_shape()[0];
    int src_stride = input.get_shape()[1];
    int dst_stride = output.get_shape()[1];

    // Broadcast współczynników
    const __m256 k00 = _mm256_set1_ps(kern[0]);
    const __m256 k01 = _mm256_set1_ps(kern[1]);
    const __m256 k02 = _mm256_set1_ps(kern[2]);
    const __m256 k10 = _mm256_set1_ps(kern[3]);
    const __m256 k11 = _mm256_set1_ps(kern[4]);
    const __m256 k12 = _mm256_set1_ps(kern[5]);
    const __m256 k20 = _mm256_set1_ps(kern[6]);
    const __m256 k21 = _mm256_set1_ps(kern[7]);
    const __m256 k22 = _mm256_set1_ps(kern[8]);

    // Przetwarzamy wiersze [1 .. height-2]
    for (int y = 1; y <= height - 2; ++y) {
        const float* r0 = src + (y - 1) * src_stride;
        const float* r1 = src + (y + 0) * src_stride;
        const float* r2 = src + (y + 1) * src_stride;

        float* drow = dst + y * dst_stride;

        // Kolumny [1 .. width-2]
        int x = 1;

        // Wektor: możemy załadować trójki: (x-1), x, (x+1)
        // tak długo, jak (x+1)+7 <= width-1  => x <= width-9
        const int xVecEnd = width - 9;

        for (; x <= xVecEnd; x += 8) {
            // r0
            __m256 r0l = _mm256_loadu_ps(r0 + x - 1);
            __m256 r0m = _mm256_loadu_ps(r0 + x + 0);
            __m256 r0r = _mm256_loadu_ps(r0 + x + 1);
            // r1
            __m256 r1l = _mm256_loadu_ps(r1 + x - 1);
            __m256 r1m = _mm256_loadu_ps(r1 + x + 0);
            __m256 r1r = _mm256_loadu_ps(r1 + x + 1);
            // r2
            __m256 r2l = _mm256_loadu_ps(r2 + x - 1);
            __m256 r2m = _mm256_loadu_ps(r2 + x + 0);
            __m256 r2r = _mm256_loadu_ps(r2 + x + 1);

            __m256 acc =
                _mm256_fmadd_ps(r0l, k00,
                _mm256_fmadd_ps(r0m, k01,
                _mm256_fmadd_ps(r0r, k02,
                _mm256_fmadd_ps(r1l, k10,
                _mm256_fmadd_ps(r1m, k11,
                _mm256_fmadd_ps(r1r, k12,
                _mm256_fmadd_ps(r2l, k20,
                _mm256_fmadd_ps(r2m, k21,
                _mm256_mul_ps (r2r, k22)))))))));

            _mm256_storeu_ps(drow + x, acc);
        }

        // Ogon skalarowy dla x = (xVecEnd+8) .. (width-2)
        for (; x <= width - 2; ++x) {
            float sum =
                r0[x - 1] * kern[0] + r0[x] * kern[1] + r0[x + 1] * kern[2] +
                r1[x - 1] * kern[3] + r1[x] * kern[4] + r1[x + 1] * kern[5] +
                r2[x - 1] * kern[6] + r2[x] * kern[7] + r2[x + 1] * kern[8];

            drow[x] = sum;
        }
    }
}

template <typename Matrix>
void conv2d_avx(const Matrix& input, const Matrix& kernel, Matrix& output) 
{
    const auto n_rows = input.get_shape()[0];
    const auto n_cols = input.get_shape()[1];

    auto inp = input.view();
    auto outp = output.view();
    auto kern = kernel.view();

    for (unsigned i = 1; i < n_rows - 1; ++i) {
        for (unsigned j = 1; j < n_cols - 1; ++j) {
            float sum = 0.0f;
            for (int ki = -1; ki <= 1; ++ki) {
                for (int kj = -1; kj <= 1; ++kj) {
                    sum += inp[i + ki][j + kj] * kern[ki + 1][kj + 1];
                }
            }
            outp[i][j] = sum;
        }
    }
}
}