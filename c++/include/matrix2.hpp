#pragma once
#include <cstdlib>
#include <algorithm>

template <typename T>
struct Matrix {
    size_t rows, cols;
    T* data {nullptr};

    Matrix(size_t r, size_t c) : rows(r), cols(c) {
        data = reinterpret_cast<T*>(std::aligned_alloc(64, rows * cols * sizeof(T)));
    }

    ~Matrix() {
        std::free(data);
    }

    static Matrix<T> empty(size_t r, size_t c) {
        return Matrix<T>(r, c);
    }

    static Matrix<T> zeros(size_t r, size_t c) {
        auto m = Matrix<T>(r, c);
        std::fill(m.data, m.data + r * c, T(0));
        return m;
    }
    static Matrix<T> ones(size_t r, size_t c) {
        auto m = Matrix<T>(r, c);
        std::fill(m.data, m.data + r * c, T(1));
        return m;
    }

    static Matrix<T> randu(size_t r, size_t c, size_t seed=0);
    static Matrix<T> randn(size_t r, size_t c, size_t seed=0);

    Matrix(Matrix&& other) noexcept : rows(other.rows), cols(other.cols), data(other.data) {
        other.data = nullptr;
        other.rows = other.cols = 0;
    }

    Matrix& operator=(Matrix&& other) noexcept {
        if (this != &other) {
            std::free(data);
            data = other.data;
            rows = other.rows;
            cols = other.cols;
            other.data = nullptr;
            other.rows = other.cols = 0;
        }
        return *this;
    }

    Matrix(const Matrix&) = delete;
    Matrix& operator=(const Matrix&) = delete;

    Matrix copy() const {
        Matrix<T> m(rows, cols);
        std::copy(data, data + rows * cols, m.data);
        return m;
    }
    
    Matrix similar() const {
        return Matrix<T>::empty(rows, cols);
    }

    T& operator()(size_t i, size_t j) {
        return data[i * cols + j];
    }

    const T& operator()(size_t i, size_t j) const {
        return data[i * cols + j];
    }
};
