module GrayScottSim

export run_simulation, parse_commandline, initialize_UV, step!, conv_laplacian, avx_laplacian

using ArgParse
using GLMakie
using LoopVectorization

function conv_laplacian(in::AbstractMatrix, out::AbstractMatrix)
    # Define weights
    c = -1.0   # center
    a = 0.2    # adjacent (up, down, left, right)
    d = 0.05   # diagonals

    # Periodic shifts
    up    = circshift(in, (-1, 0))
    down  = circshift(in, (1, 0))
    left  = circshift(in, (0, -1))
    right = circshift(in, (0, 1))

    up_left    = circshift(in, (-1, -1))
    up_right   = circshift(in, (-1, 1))
    down_left  = circshift(in, (1, -1))
    down_right = circshift(in, (1, 1))

    # Combine with weights
    out .= c .* in .+
           a .* (up .+ down .+ left .+ right) .+
           d .* (up_left .+ up_right .+ down_left .+ down_right)
end

function imfilter_laplacian(in::AbstractMatrix, out::AbstractMatrix)
    kx = [0.05, 0.2, 0.05]
    ky = [0.05, 0.2, 0.05]
    center = -1.0

    smooth = imfilter(in, (ky, kx), "circular")   # 1D conv in y, then x
    out .= smooth .+ center .* in
end

function avx_laplacian(in::AbstractMatrix, out::AbstractMatrix)
    k = [0.05 0.2 0.05;
         0.2 -1.0 0.2;
         0.05 0.2 0.05]
    nx, ny = size(in)
    @turbo for i in 2:nx-1
        for j in 2:ny-1
            s = 0.0f0
            for dy in -1:1, dx in -1:1
                s += k[dy+2, dx+2] * in[i+dy, j+dx]
            end
            out[i,j] = s
        end
    end
end

function step!(U::AbstractMatrix{T}, V::AbstractMatrix{T}, 
                         U_new::AbstractMatrix{T}, V_new::AbstractMatrix{T},
                         U_lap::AbstractMatrix{T}, V_lap::AbstractMatrix{T},
                         params, dt::T) where T<:AbstractFloat
    Du, Dv, F, k = params.du, params.dv, params.F, params.k
    
    avx_laplacian(U,U_lap)
    avx_laplacian(V,V_lap)

    # Reaction term
    uvv = U .* (V .^ 2)

    # Update equations
    @turbo U_new .= U .+ (Du .* U_lap .- uvv .+ F .* (1 .- U)) .* dt
    @turbo V_new .= V .+ (Dv .* V_lap .+ uvv .- (F .+ k) .* V) .* dt
end

function parse_commandline()
    s = ArgParseSettings()
    @add_arg_table s begin
        "--nx"
            help = "Number of grid points in x direction"
            arg_type = Int
            default = 100
        "--ny"
            help = "Number of grid points in y direction"
            arg_type = Int
            default = 100
        "--ns"
            help = "Seed square size"
            arg_type = Int
            default = 10
        "--rand"
            help = "Randomize initial condition"
            arg_type = Float32
            default = 0.02
        "--du"
            help = "Diffusion coefficient for U"
            arg_type = Float32
            default = 0.16
        "--dv"
            help = "Diffusion coefficient for V"
            arg_type = Float32
            default = 0.08
        "--F"
            help = "Feed rate"
            arg_type = Float32
            default = 0.0367
        "--k"
            help = "Kill rate"
            arg_type = Float32
            default = 0.0649
        "--dt"
            help = "Time step"
            arg_type = Float32
            default = 1.0
        "--steps"
            help = "Number of simulation steps"
            arg_type = Union{Int,Nothing}
            default = nothing
        "--fps"
            help = "Frames per second for visualization"
            arg_type = Int
            default = 20
    end
    args = parse_args(s)
    (; (Symbol(k) => v for (k, v) in args)...)
end

function initialize_UV(params)
    Nx, Ny, Ns = params.nx, params.ny, params.ns
    rnd = params.rand

    U =  ones(Float32, Nx, Ny)
    V = zeros(Float32, Nx, Ny)
    
    # Initial condition: small square in the center
    cx, cy = div(Nx,2), div(Ny,2)
    U[cx-Ns:cx+Ns, cy-Ns:cy+Ns] .= 0.50f0
    V[cx-Ns:cx+Ns, cy-Ns:cy+Ns] .= 0.25f0
    
    # Add some random noise
    if rnd > 0.0
        U .+= rnd * (rand(Float32, Nx, Ny) .- 0.5f0)
        V .+= rnd * (rand(Float32, Nx, Ny) .- 0.5f0)
    end
    U, V
end

function show_until_closed(fig)
    display(fig)
    while isopen(fig.scene)
        yield()     # give time to the render/event loop
        sleep(0.05) # small nap to avoid busy-wait
    end
end

function show_until_closed_2(fig)
    wait(display(fig))
end

function show_until_closed_3(fig)
end

function run_simulation(params)
    dt = params.dt
    nsteps = params.steps
    frame_ns = 1e9 / params.fps

    U, V = initialize_UV(params)
    U_new, V_new = similar(U), similar(V)
    U_lap, V_lap = similar(U), similar(V)

    set_theme!(theme_black())
    fig = Figure(size = (900, 900))
    ax  = Axis(fig[1, 1], title = "Gray-Scott (F=$(params.F), k=$(params.k))", xlabel="x", ylabel="y")
    Vshow = copy(V)
    data  = Observable(Vshow)
    hm = heatmap!(ax, data; interpolate=false, colorrange=(0.0, 1.0))
    Colorbar(fig[1, 2], hm, label = "V")
    screen = display(fig)

    frame_no = 1
    while isopen(screen)    
        t = time_ns()
        for _ in 1:50
            step!(U, V, U_new, V_new, U_lap, V_lap, params, dt)
        end
        st = (time_ns() - t) / 1e3
        ax.title[] = "Frame $frame_no step time: $(round(st, digits=2)) us"
        @inbounds Vshow .= V   # zaktualizuj bufor wizualizacji bez alokacji
        notify(data)           # poinformuj Makie, że dane się zmieniły
        yield()                # oddaj sterowanie, by Makie mógł odrysować
        tsleep = (frame_ns - (time_ns() - t)) / 1e9
        tsleep < 0 || sleep(tsleep)
        frame_no += 1
        if nsteps !== nothing && frame_no > nsteps
            yield()
            break
        end
        U, U_new = U_new, U
        V, V_new = V_new, V
    end
end

end # module GrayScottSim
