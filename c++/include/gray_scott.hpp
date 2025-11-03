#pragma once
#include <utility>
#include <optional>
#include <memory>
#include <string>

namespace GrayScott {

using Float32 = float;

struct Params {
    Float32 Du; // Diffusion rate of U
    Float32 Dv; // Diffusion rate of V
    Float32 F;  // Feed rate
    Float32 k;  // Kill rate
    Float32 dt; // Time step
    Float32 initial_noise;
    unsigned Nx, Ny, Ns;

    std::optional<unsigned> seed;
    std::optional<unsigned> Nsteps;
    unsigned fps;
};
struct Backend
{
    virtual bool initialize(const Params&) = 0;
    virtual void gray_scott_step(float dt) = 0;
    // assuming width and height the same as in Params
    virtual void copy_to_output(void* output, int format) = 0; 
    static std::unique_ptr<Backend> create(const std::string& type);
};

} // namespace GrayScott
