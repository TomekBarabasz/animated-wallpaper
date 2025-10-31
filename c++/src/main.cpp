#include <iostream>
#include <matrix.hpp>
//#include <gray_scott.hpp>
#include <Eigen/Dense>

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

int main(int argc, char* argv[]) 
{
    std::cout << "Gray-Scott Simulation" << std::endl;

    test_matrix();
    test_eigen();

    return 0;
}


