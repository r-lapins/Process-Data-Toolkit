#pragma once

#include "pdt/compute/ifft_backend.h"

namespace pdt {

class CudaFftBackend final : public IFftBackend {
  public:
    const char* name()  const noexcept override { return "CUDA"; }
    bool is_gpu()       const noexcept override { return true; }

    FftComputationResult compute_spectrum(std::span<const double> signal, double sample_rate, WindowType window) override;
};

} // namespace pdt
