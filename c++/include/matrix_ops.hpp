#include <matrix.hpp>

namespace matrix::ops {

using matrix::Matrix;
void conv3x3_f32(const Matrix<float,2>& input, const Matrix<float,2>& kernel, Matrix<float,2>& output);
void conv3x3_f32_avx2(const Matrix<float,2>& input, const Matrix<float,2>& kernel, Matrix<float,2>& output);

} // namespace matrix::ops
