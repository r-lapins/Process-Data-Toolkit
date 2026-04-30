#pragma once

#include "pdt/compute/ifft_backend.h"

#include <complex>

namespace pdt {

class CpuFftBackend final : public IFftBackend {
  public:
    const char* name()  const noexcept override { return "CPU"; }
    bool is_gpu()       const noexcept override { return false; }

    FftComputationResult compute_spectrum(std::span<const double> signal,
                                          double sample_rate,
                                          WindowType window) override;

    FftComputationResult compute_iq_spectrum(std::span<const std::complex<float>> iq,
                                             double sample_rate,
                                             WindowType window) override;
};

} // namespace pdt
