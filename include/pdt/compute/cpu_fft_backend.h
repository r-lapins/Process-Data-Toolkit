#pragma once

#include "pdt/compute/ifft_backend.h"

namespace pdt {

class CpuFftBackend final : public IFftBackend {
  public:
    const char* name()  const noexcept override { return "CPU"; }
    bool is_gpu()       const noexcept override { return false; }

    FftComputationResult compute_spectrum(std::span<const double> signal, double sample_rate, WindowType window) override;
};

} // namespace pdt
