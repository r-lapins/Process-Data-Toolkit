#include "pdt/compute/cuda_fft_backend.h"

#include "pdt/dsp/window.h"

#include <cuda_runtime.h>
#include <cufft.h>

#include <cmath>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

namespace pdt {
namespace {

const char* cufft_result_to_string(cufftResult r)
{
    switch (r) {
    case CUFFT_SUCCESS: return "CUFFT_SUCCESS";
    case CUFFT_INVALID_PLAN: return "CUFFT_INVALID_PLAN";
    case CUFFT_ALLOC_FAILED: return "CUFFT_ALLOC_FAILED";
    case CUFFT_INVALID_TYPE: return "CUFFT_INVALID_TYPE";
    case CUFFT_INVALID_VALUE: return "CUFFT_INVALID_VALUE";
    case CUFFT_INTERNAL_ERROR: return "CUFFT_INTERNAL_ERROR";
    case CUFFT_EXEC_FAILED: return "CUFFT_EXEC_FAILED";
    case CUFFT_SETUP_FAILED: return "CUFFT_SETUP_FAILED";
    case CUFFT_INVALID_SIZE: return "CUFFT_INVALID_SIZE";
    case CUFFT_NOT_SUPPORTED: return "CUFFT_NOT_SUPPORTED";
    default: return "CUFFT_UNKNOWN";
    }
}

void check_cuda(cudaError_t err, const char* what)
{
    if (err != cudaSuccess) { throw std::runtime_error(std::string(what) + ": " + cudaGetErrorString(err)); }
}

void check_cufft(cufftResult err, const char* what)
{
    if (err != CUFFT_SUCCESS) { throw std::runtime_error(std::string(what) + ": " + cufft_result_to_string(err)); }
}

struct CudaFreeDeleter {
    void operator()(void* p) const noexcept
    {
        if (p != nullptr) { cudaFree(p); }
    }
};

} // namespace

FftComputationResult CudaFftBackend::compute_spectrum(std::span<const double> signal, double sample_rate, WindowType window) {
    if (signal.empty()) { return {}; }

    const auto windowed = apply_window(signal, window);
    const std::size_t N = windowed.size();

    if (N < 2) { return {}; }

    // convert to float (GPU friendly)
    std::vector<float> input(N);
    for (std::size_t i = 0; i < N; ++i) {
        input[i] = static_cast<float>(windowed[i]);
    }

    // plan R2C (Real to Complex)
    cufftHandle plan{};
    check_cuda(cudaSetDevice(0), "cudaSetDevice");
    check_cuda(cudaFree(nullptr), "cudaFree(nullptr)");
    check_cufft(cufftPlan1d(&plan, static_cast<int>(N), CUFFT_R2C, 1), "cufftPlan1d");

    // GPU buffers
    float* raw_input = nullptr;
    cufftComplex* raw_output = nullptr;

    check_cuda(cudaMalloc(&raw_input, N * sizeof(float)), "cudaMalloc d_input");
    check_cuda(cudaMalloc(&raw_output, (N / 2 + 1) * sizeof(cufftComplex)), "cudaMalloc d_output");

    std::unique_ptr<float, CudaFreeDeleter> d_input(raw_input);
    std::unique_ptr<cufftComplex, CudaFreeDeleter> d_output(raw_output);

    // copy data to GPU
    check_cuda(cudaMemcpy(d_input.get(), input.data(), N * sizeof(float), cudaMemcpyHostToDevice)
               , "cudaMemcpy H2D");

    check_cufft(cufftExecR2C(plan, d_input.get(), d_output.get())
                , "cufftExecR2C");
    check_cuda(cudaDeviceSynchronize()
               , "cudaDeviceSynchronize");

    // copy result to CPU
    std::vector<cufftComplex> output((N / 2) + 1);
    check_cuda(cudaMemcpy(output.data(), d_output.get(), ((N / 2) + 1) * sizeof(cufftComplex), cudaMemcpyDeviceToHost)
               , "cudaMemcpy D2H");
    cufftDestroy(plan);

    // build Spectrum
    Spectrum spectrum;
    spectrum.frequencies.reserve((N/2) + 1);
    spectrum.magnitudes.reserve((N/2) + 1);

    for (std::size_t k = 0; k <= N/2; ++k) {
        const double freq = static_cast<double>(k) * sample_rate / static_cast<double>(N);

        const float re = output[k].x;
        const float im = output[k].y;
        const double mag = std::sqrt((re * re) + (im * im));

        spectrum.frequencies.push_back(freq);
        spectrum.magnitudes.push_back(mag);
    }

    return {
        .spectrum = std::move(spectrum),
        .algorithm = SpectrumAlgorithm::cuFft
    };
}

} // namespace pdt
