
#include <gray_scott.hpp>
#include <matrix.hpp>
#include <cstring>

using Float32 = float;
using MatrixF32 = matrix::Matrix<Float32, 2>;

namespace GrayScott {
struct NaiveBackend : public Backend
{
    MatrixF32 U, V, U_new, V_new, U_lap, V_lap, lap_kernel;
    Params params;
    std::pair<MatrixF32, MatrixF32> initialize_UV(const Params& params)
    {
        auto U = matrix::ones<float>(params.Nx, params.Ny);
        auto V = matrix::zeros<float>(params.Nx, params.Ny);

        auto initial_noise = params.initial_noise;
        srand(params.seed.value_or(0));

        // Initialize U and V with some pattern plus noise
        auto vU = U.view();
        auto vV = V.view();
        for (unsigned i = 0; i < params.Nx; ++i) {
            for (unsigned j = 0; j < params.Ny; ++j) {
                vU[i][j] = 1.0 + params.initial_noise * ((double)rand() / RAND_MAX - 0.5);
                vV[i][j] = params.initial_noise * ((double)rand() / RAND_MAX - 0.5);
            }
        }
        return {std::move(U), std::move(V)};
    }

    bool initialize(const Params& params) override
    {
        std::tie(U, V) = initialize_UV(params);
        U_new = U.similar();
        V_new = V.similar();
        U_lap = U.similar();
        V_lap = V.similar();
        lap_kernel = matrix::empty<float>(3, 3);

        const float kernel[]  = { 
            .05f, .02f, .05f,
            .2f, -1, .2f,
            .05f, .2f, .05f
        };
        memcpy(lap_kernel.get_data(), kernel, sizeof(kernel));
        this->params = params;
        return true;
    }

    void conv2d(const MatrixF32& input, const MatrixF32& kernel, MatrixF32& output)
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

    void gray_scott_step(float dt) override
    {
        conv2d(U, lap_kernel, U_lap);
        conv2d(V, lap_kernel, V_lap);
        auto u_view = U.view();
        auto v_view = V.view();
        auto u_new_view = U_new.view();
        auto v_new_view = V_new.view();
        auto u_lap_view = U_lap.view();
        auto v_lap_view = V_lap.view();

        for (unsigned i = 0; i < params.Nx; ++i) {
            for (unsigned j = 0; j < params.Ny; ++j) {
                float u = u_view[i][j];
                float v = v_view[i][j];
                float uvv = u * v * v;
                float du = params.Du * u_lap_view[i][j] - uvv + params.F * (1 - u);
                float dv = params.Dv * v_lap_view[i][j] + uvv - (params.F + params.k) * v;
                u_new_view[i][j] = u + du * dt;
                v_new_view[i][j] = v + dv * dt;
            }
        }
    }

    void copy_to_output(void* output, int format) override
    {
        // Implement the copy to output functionality
    }
};

struct AVX256Backend : public NaiveBackend 
{
    void gray_scott_step(float dt) override
    {
    }
    void copy_to_output(void* output, int format) override
    {
        // Implement the copy to output functionality
    }
};

std::unique_ptr<Backend> Backend::create(const std::string& type)
{
    if (type == "naive") {
        return std::make_unique<NaiveBackend>();
    }
    else if (type == "avx256") {
        return std::make_unique<AVX256Backend>();
    }
    //else if (type == "cuda") {
    //    return new GrayScottBackendCUDA();
    //}  
    
    return nullptr;
}

} // namespace GrayScott
