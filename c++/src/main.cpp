#include <iostream>
#include <matrix.hpp>
//#include <gray_scott.hpp>
#include <Eigen/Dense>
#include <profiler.hpp>

using namespace matrix;

using Matrix2f32 = matrix::Matrix<float, 2>;
using Matrix3f32 = matrix::Matrix<float, 3>;

void test_matrix()
{
    auto mat1 = matrix::empty<float>(10,20);

    Matrix2f32 mat(100, 100);
    mat({0, 0}) = 1.0f;

    // Copy the matrix
    auto mat_copy = mat.copy();

    // Create a similar matrix
    auto mat_similar = mat.similar();

    auto _mat3 = matrix::empty<float>(1, 2, 3);

    auto mat3 = matrix::zeros<float>(10, 15, 20);
    mat3({0, 1, 2}) = 1.0f;

    auto & shape = mat3.get_shape();
    std::cout << "Matrix3 shape: ";
    for (auto dim : shape) {
        std::cout << dim << " ";
    }
    std::cout << std::endl;
    std::cout << "Matrix3 total size: " << mat3.total_size() << std::endl;
    std::cout << "Matrix3 total bytes: " << mat3.total_bytes() << std::endl;

    std::cout << "sizeof(Matrix2f32): " << sizeof(Matrix2f32) << std::endl;
    std::cout << "sizeof(Matrix3f32): " << sizeof(Matrix3f32) << std::endl;

    auto v = mat3.view();
    v[0][1][2] = 3.14f;
    std::cout << "mat3(0,1,2) = " << v[0][1][2] << std::endl;

    mat3[0][1][2] = 31.4f;
    std::cout << "mat3(0,1,2) = " << mat3[0][1][2] << std::endl;
}

void test_eigen()
{
    using Eigen::MatrixXf;
    MatrixXf mat(100, 100);
    mat(0, 0) = 1.0f;

    MatrixXf mat_copy = mat;

    MatrixXf mat_similar = MatrixXf::Zero(mat.rows(), mat.cols());

    // these are const matrices
    auto m3 = MatrixXf::Random(10, 15);
    auto m4 = MatrixXf::Zero(10, 15);
    // m4(0, 1) = 1.0f; // error: assignment of read-only location

    mat.setRandom();
    mat.setZero();
}

void conv3x3_f32(const Matrix<float,2>& input, const Matrix<float,2>& kernel, Matrix<float,2>& output);
void conv3x3_f32_avx2(const Matrix<float,2>& input, const Matrix<float,2>& kernel, Matrix<float,2>& output);
void test_conv()
{
    size_t n = 10;
    auto A = matrix::randu<float>(n,n);
    auto B1 = matrix::zeros<float>(A.get_shape());
    auto B2 = matrix::zeros<float>(n,n);
    auto K = matrix::zeros<float>(3,3);
    float kernel_data[] = {
        0.05f, .20f, 0.05f,
        .02f, -1.0f, .2f,
        0.05f, .2f, 0.05f
    };
    memcpy(K.get_data(), kernel_data, sizeof(kernel_data));

    conv3x3_f32(A, K, B1);
    conv3x3_f32_avx2(A, K, B2);

    const auto is_same = B1 == B2;
    const auto almost_equal = matrix::almost_equal(B1,B2);

    std::cout << "conv3x3_f32 scalar vs avx : is_same :" << (is_same ? "YES":"NO") 
              << " is almost equal : " << (almost_equal ? "YES":"NO") << std::endl;
    
    constexpr size_t N_RUNS = 1000;
    Profiler p;
    { 
        Profiler::Section section(p, "conv3x3_f32");
        for (int i=0;i<N_RUNS;++i) {
            conv3x3_f32(A, K, B1);
        }
    }
    { 
        Profiler::Section section(p, "conv3x3_f32_avx2");
        for (int i=0;i<N_RUNS;++i) {
            conv3x3_f32_avx2(A, K, B2);
        }
    }
    auto measurements = p.get_measurements("us");
    for (const auto& [k,v] : measurements) {
        std::cout << k << ": " << median(v)/N_RUNS<< " us over " << N_RUNS << " runs" << std::endl;
    }
}

int main(int argc, char* argv[]) 
{
    std::cout << "Gray-Scott Simulation" << std::endl;

    test_matrix();
    test_eigen();
    test_conv();

    return 0;
}


