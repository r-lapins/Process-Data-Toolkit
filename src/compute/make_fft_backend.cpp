#include "pdt/compute/make_fft_backend.h"

#include "pdt/compute/cpu_fft_backend.h"

#ifdef PDT_ENABLE_CUDA
#include "pdt/compute/cuda_fft_backend.h"
#endif

#include <stdexcept>

namespace pdt {

bool is_gpu_algorithm(SpectrumAlgorithm algorithm) noexcept
{
    return algorithm == SpectrumAlgorithm::cuFft;
}

bool is_algorithm_available(SpectrumAlgorithm algorithm) noexcept
{
    switch (algorithm) {
    case SpectrumAlgorithm::Auto:
    case SpectrumAlgorithm::Dft:
    case SpectrumAlgorithm::Fft:
        return true;

    case SpectrumAlgorithm::cuFft:
#ifdef PDT_ENABLE_CUDA
        return true;
#else
        return false;
#endif
    }

    return false;
}

std::unique_ptr<IFftBackend> create_fft_backend(SpectrumAlgorithm algorithm)
{
    switch (algorithm) {
    case SpectrumAlgorithm::Auto:
    case SpectrumAlgorithm::Dft:
    case SpectrumAlgorithm::Fft:
        return std::make_unique<CpuFftBackend>();

    case SpectrumAlgorithm::cuFft:
#ifdef PDT_ENABLE_CUDA
        return std::make_unique<CudaFftBackend>();
#else
        throw std::runtime_error("CUDA backend requested, but PDT was built without CUDA support.");
#endif
    }

    throw std::runtime_error("Unsupported spectrum algorithm.");
}

} // namespace pdt
