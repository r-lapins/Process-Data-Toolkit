#include "pdt/compute/cuda_fft_backend.h"

#include "pdt/dsp/window.h"

#include <cuda_runtime.h>
#include <cufft.h>

#include <cmath>
#include <cstddef>
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

template <typename T>
struct CudaDeleter {
    void operator()(T* p) const noexcept
    {
        if (p != nullptr) { cudaFree(p); }
    }
};

using DeviceFloatPtr    = std::unique_ptr<float, CudaDeleter<float>>;
using DeviceComplexPtr  = std::unique_ptr<cufftComplex, CudaDeleter<cufftComplex>>;
using DeviceBytePtr     = std::unique_ptr<std::byte, CudaDeleter<std::byte>>;

} // namespace

// Reusable CUDA/cuFFT state cached per transform size
// Avoids re-creating the FFT plan and GPU buffers on every call
struct CudaFftBackend::Impl {
    int device_id{0};

    // Current FFT size for which plan, workspace and buffers are prepared
    std::size_t planned_n{0};
    std::size_t output_size{0};

    cufftHandle plan{};
    bool plan_ready{false};

    std::size_t work_size_bytes{0};
    DeviceBytePtr work_area{nullptr};

    DeviceFloatPtr d_input{nullptr};
    DeviceComplexPtr d_output{nullptr};

    std::vector<float> host_input;
    std::vector<cufftComplex> host_output;

    ~Impl() { if (plan_ready) { cufftDestroy(plan); } }

    // Ensures that plan, work area and device/host buffers are allocated
    // for the requested FFT size. Rebuilds cached state only when N changes.
    void ensure_ready(std::size_t n) {
        if (plan_ready && planned_n == n) { return; }

        release_resources();

        planned_n = n;
        output_size = (n / 2) + 1;

        check_cuda(cudaSetDevice(device_id), "cudaSetDevice");
        check_cuda(cudaFree(nullptr), "cudaFree(nullptr)");

        float* raw_input = nullptr;
        cufftComplex* raw_output = nullptr;

        check_cuda(cudaMalloc(&raw_input, n * sizeof(float)), "cudaMalloc d_input");
        check_cuda(cudaMalloc(&raw_output, output_size * sizeof(cufftComplex)), "cudaMalloc d_output");

        d_input.reset(raw_input);
        d_output.reset(raw_output);

        host_input.resize(n);
        host_output.resize(output_size);

        check_cufft(cufftCreate(&plan), "cufftCreate");
        plan_ready = true;

        check_cufft(cufftSetAutoAllocation(plan, 0), "cufftSetAutoAllocation");
        check_cufft(cufftMakePlan1d(plan, static_cast<int>(n), CUFFT_R2C, 1, &work_size_bytes), "cufftMakePlan1d");

        std::byte* raw_work = nullptr;
        check_cuda(cudaMalloc(reinterpret_cast<void**>(&raw_work), work_size_bytes), "cudaMalloc work_area");
        work_area.reset(raw_work);

        check_cufft(cufftSetWorkArea(plan, work_area.get()), "cufftSetWorkArea");
    }

    // Releases all cached CUDA/cuFFT resources associated with the current FFT size
    void release_resources() {
        if (plan_ready) {
            cufftDestroy(plan);
            plan_ready = false;
            plan = {};
        }

        work_area.reset(nullptr);
        d_input.reset(nullptr);
        d_output.reset(nullptr);

        work_size_bytes = 0;
        planned_n = 0;
        output_size = 0;

        host_input.clear();
        host_output.clear();
    }
};

CudaFftBackend::CudaFftBackend() : impl_(std::make_unique<Impl>()) {}
CudaFftBackend::~CudaFftBackend() = default;

FftComputationResult CudaFftBackend::compute_spectrum(std::span<const double> signal, double sample_rate, WindowType window) {
    if (signal.empty()) { return {}; }

    const auto windowed = apply_window(signal, window);
    const std::size_t N = windowed.size();

    if (N < 2) { return {}; }

    impl_->ensure_ready(N);

    // Convert to float (GPU friendly)
    // cuFFT is used here as a fast FFT accelerator; the surrounding pipeline remains CPU-centric.
    for (std::size_t i = 0; i < N; ++i) {
        impl_->host_input[i] = static_cast<float>(windowed[i]);
    }

    check_cuda(
        cudaMemcpy(impl_->d_input.get(), impl_->host_input.data(),
                   N * sizeof(float), cudaMemcpyHostToDevice)
        , "cudaMemcpy H2D");
    check_cufft(
        cufftExecR2C(impl_->plan, impl_->d_input.get(), impl_->d_output.get())
        , "cufftExecR2C");
    check_cuda(
        cudaMemcpy(impl_->host_output.data(), impl_->d_output.get(),
                   impl_->output_size * sizeof(cufftComplex), cudaMemcpyDeviceToHost)
        , "cudaMemcpy D2H");

    // build Spectrum
    Spectrum spectrum;
    spectrum.frequencies.reserve(impl_->output_size);
    spectrum.magnitudes.reserve(impl_->output_size);

    for (std::size_t k = 0; k < impl_->output_size; ++k) {
        const float re = impl_->host_output[k].x;
        const float im = impl_->host_output[k].y;

        const double freq = static_cast<double>(k) * sample_rate / static_cast<double>(N);
        const double mag = std::sqrt((re * re) + (im * im));

        spectrum.frequencies.push_back(freq);
        spectrum.magnitudes.push_back(mag);
    }

    return {
        .spectrum = std::move(spectrum),
        .algorithm = SpectrumAlgorithm::cuFft
    };
}

FftComputationResult CudaFftBackend::compute_iq_spectrum(std::span<const std::complex<float>>,
                                                         double,
                                                         WindowType) {
    throw std::runtime_error("CUDA IQ spectrum is not implemented yet");
}

} // namespace pdt
