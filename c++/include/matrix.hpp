#pragma once
#include <cstdlib>
#include <algorithm>
#include <memory>
#include <array>
#include <cassert>

namespace matrix {

template <typename T, std::size_t N>
struct View;

template <typename T, int N>
struct Matrix {
    using Shape = std::array<uint16_t, N>;
    using Stride = std::array<std::size_t, N>;
    using Position = Shape;

    size_t total_size() const {
        size_t size = 1;
        for (size_t dim : shape) size *= dim;
        return size;
    }

    size_t total_bytes() const {
        return total_size() * sizeof(T);
    }

    void allocate() {
        data.reset(static_cast<T*>(std::aligned_alloc(64, total_bytes())));
    }

    std::array<std::size_t, N> make_row_major_strides(const Shape& shape) const {
        std::array<std::size_t, N> s{};
        std::size_t acc = 1;
        for (std::size_t d = N; d-- > 0; ) { s[d] = acc; acc *= shape[d]; }
        return s;
    }

    explicit Matrix(const Shape& shape) : shape(shape)
    {
        allocate();
    }

    template <typename... Dims>
    explicit Matrix(Dims... dims) 
        : shape{{static_cast<uint16_t>(dims)...}}
    {
        allocate();
    }

    ~Matrix() {
        // No need to free data, unique_ptr will handle it
    }

    static Matrix<T, N> empty(Shape shape) {
        return Matrix<T,N>(shape);
    }

    static Matrix<T, N> zeros(Shape shape) {
        return Matrix<T,N>::fill(shape, T(0));
    }

    static Matrix<T, N> ones(Shape shape) {
        return Matrix<T,N>::fill(shape, T(1));
    }

    static Matrix<T, N> fill(Shape shape, T value) {
        auto m = Matrix<T,N>(shape);
        std::fill_n(m.data.get(), m.total_size(), value);
        return m;
    }

    Matrix(Matrix&& other) noexcept 
        : shape(other.shape), 
        data(std::move(other.data)) {
        other.shape.fill(0);
    }

    Matrix& operator=(Matrix&& other) noexcept {
        if (this != &other) {
            data = std::move(other.data);
            shape = other.shape;
            other.shape.fill(0);
        }
        return *this;
    }

    Matrix(const Matrix&) = delete;

    Matrix& operator=(const Matrix&) = delete;

    Matrix copy() const {
        Matrix<T, N> m(shape);
        auto ptr = data.get();
        std::copy(ptr, ptr + total_size(), m.data.get());
        return m;
    }
    
    Matrix similar() const {
        return Matrix<T, N>::empty(shape);
    }

    static size_t offset(const Shape& shape, const Position& pos) {
        size_t offset = 0;
        size_t stride = 1;
        for (int i = N - 1; i >= 0; --i) {
            offset += pos[i] * stride;
            stride *= shape[i];
        }
        return offset;
    }
    
    T& operator()(Position pos) {
        return data[offset(shape, pos)];
    }
    
    const T& operator()(Position pos) const {
        return data[offset(shape, pos)];
    }
    
    View<T, N-1> operator[](std::size_t i) const {
        static_assert(N >= 2, "Use View<T,1> directly for rank-1.");
        assert(i < shape[0]);
        View<T, N-1> v{};
        auto stride = make_row_major_strides(shape);
        v.ptr = data.get() + i * stride[0];
        for (std::size_t d = 1; d < N; ++d) {
            v.shape[d-1]  = shape[d];
            v.stride[d-1] = stride[d];
        }
        return v;
    }

    View<T, N> view() {
        View<T, N> v{};
        v.ptr = data.get();
        v.shape = shape;
        v.stride = make_row_major_strides(shape);
        return v;
    }

    View<const T, N> view() const {
        View<const T, N> v{};
        v.ptr = data.get();
        v.shape = shape;
        v.stride = make_row_major_strides(shape);
        return v;
    }

    constexpr size_t get_ndims() const {
        return N;
    }
    
    const Shape& get_shape() const {
        return shape;
    }
    
    const T* get_data() const {
        return data.get();
    }

    T* get_data() {
        return data.get();
    }

    typedef T value_type;

protected:
    struct Deleter {
        void operator()(T* ptr) {
            std::free(ptr);
        }
    };
    Shape shape;
    std::unique_ptr<T[], Deleter> data {nullptr};
};

template <typename T, typename... Dims>
auto empty(Dims... dims) 
{
    constexpr size_t N = sizeof...(Dims);
    using Mat = Matrix<T, N>;
    return Mat::empty(typename Mat::Shape{static_cast<uint16_t>(dims)...});
}

template <typename T, typename... Dims>
auto zeros(Dims... dims) 
{
    constexpr size_t N = sizeof...(Dims);
    using Mat = Matrix<T, N>;
    return Mat::zeros(typename Mat::Shape{static_cast<uint16_t>(dims)...});
}

template <typename T, typename... Dims>
auto ones(Dims... dims) 
{
    constexpr size_t N = sizeof...(Dims);
    using Mat = Matrix<T, N>;
    return Mat::ones(typename Mat::Shape{static_cast<uint16_t>(dims)...});
}

template <typename T, typename... Dims>
auto from_value(T value, Dims... dims) 
{
    constexpr size_t N = sizeof...(Dims);
    using Mat = Matrix<T, N>;
    return Mat::fill(typename Mat::Shape{static_cast<uint16_t>(dims)...}, value);
}

template <typename T, typename... Dims>
auto randu(Dims... dims);

template <typename T, typename... Dims>
auto randn(Dims... dims);

template <typename T>
struct View<T, 1> {
    T* ptr{};
    std::array<std::size_t, 1> shape{};
    std::array<std::size_t, 1> stride{1};

    inline T& operator[](std::size_t i) const {   
        return ptr[i];
    }
};

template <typename T, std::size_t N>
struct View {
    static_assert(N >= 2, "N must be >= 2 here");
    T* ptr{};
    typename Matrix<T, N>::Shape shape{};
    std::array<std::size_t, N> stride{};

    inline View<T, N-1> operator[](std::size_t i) const {
        assert(i < shape[0]);
        View<T, N-1> v{};
        v.ptr = ptr + i * stride[0];
        // shift shape/stride left by one
        for (std::size_t d = 1; d < N; ++d) {
            v.shape[d-1]  = shape[d];
            v.stride[d-1] = stride[d];
        }
        return v;
    }
};

} // namespace matrix
