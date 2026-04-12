#pragma once

#include "pdt/dsp/spectrum.h"
#include "pdt/dsp/window.h"

#include <span>

namespace pdt {

struct FftComputationResult {
    Spectrum spectrum;
    SpectrumAlgorithm algorithm{SpectrumAlgorithm::Auto};
};

// Interface for FFT-based spectrum computation backends (CPU / GPU)
class IFftBackend {
  public:
    virtual ~IFftBackend() = default;

    virtual const char* name()  const noexcept = 0;
    virtual bool is_gpu()       const noexcept = 0;

    virtual FftComputationResult compute_spectrum(std::span<const double> signal, double sample_rate, WindowType window) = 0;
};

} // namespace pdt
