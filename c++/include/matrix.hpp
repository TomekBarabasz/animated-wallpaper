#pragma once
#include <cstdlib>
#include <algorithm>
#include <memory>
#include <array>
#include <cassert>
#include <type_traits>
#include <cmath>

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
        return Matrix<T,N>::from_value(shape, T(0));
    }

    static Matrix<T, N> ones(Shape shape) {
        return Matrix<T,N>::from_value(shape, T(1));
    }

    static Matrix<T, N> from_value(Shape shape, T value) {
        auto m = Matrix<T,N>(shape);
        std::fill_n(m.data.get(), m.total_size(), value);
        return m;
    }

    void fill(T value){
        std::fill_n(data.get(), total_size(), value);
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

    bool operator==(const Matrix& other) const {
        if (shape != other.shape) return false;
        if constexpr (std::is_floating_point_v<T>) {
            constexpr T eps = static_cast<T>(1e-6); // or configurable
            return std::equal(
                data.get(), data.get() + total_size(), other.data.get(),
                [eps](T a, T b) { return std::fabs(a - b) <= eps; }
            );
        } else {
            // exact comparison for integers, bool, etc.
            return std::equal(
                data.get(), data.get() + total_size(), other.data.get()
            );
        }
    }

    bool operator!=(const Matrix& other) const {
        return !(*this == other);
    }

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
requires (std::conjunction_v<std::is_integral<Dims>...>)
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
    return Mat::from_value(typename Mat::Shape{static_cast<uint16_t>(dims)...}, value);
}

template <typename T, int N>
Matrix<T, N> similar(const Matrix<T, N>& mat) {
    return Matrix<T, N>::empty(mat.get_shape());
}

template <typename T, typename Shape>
auto zeros(const Shape& shape) {
    constexpr size_t N = std::tuple_size_v<Shape>;
    return Matrix<T, N>::zeros(shape);
}

template <typename T>
void randu(T* data, size_t size);

template <typename T, typename... Dims>
auto randu(Dims... dims) {
    constexpr size_t N = sizeof...(Dims);
    using Mat = Matrix<T, N>;
    auto m = Mat::empty(typename Mat::Shape{static_cast<uint16_t>(dims)...});
    randu(m.get_data(), m.total_size());
    return m;
}

template <typename T>
void randn(T* data, size_t size);

template <typename T, typename... Dims>
auto randn(Dims... dims) {
    constexpr size_t N = sizeof...(Dims);
    using Mat = Matrix<T, N>;
    auto m = Mat::empty(typename Mat::Shape{static_cast<uint16_t>(dims)...});
    randn(m.get_data(), m.total_size());
    return m;
}

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


template <typename T, int N>
bool almost_equal(const Matrix<T,N>& a, const Matrix<T,N>& b, T abs_eps = static_cast<T>(1e-6), T rel_eps = static_cast<T>(1e-5)) {
    if (a.get_shape() != b.get_shape()) return false;
    if constexpr (std::is_floating_point_v<T>) {
        return std::equal(
            a.get_data(), a.get_data() + a.total_size(), b.get_data(),
            [abs_eps, rel_eps](T a, T b) {
                const T diff = std::fabs(a - b);
                const T tol = abs_eps + rel_eps * std::max(std::fabs(a), std::fabs(b));
                return diff <= tol;
            }
        );
    } else {
        // exact comparison for integers, bool, etc.
        return std::equal(
            a.get(), a.get() + a.total_size(), b.get()
        );
    }
}

} // namespace matrix
