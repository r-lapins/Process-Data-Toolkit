#pragma once

#include "pdt/compute/ifft_backend.h"
#include "pdt/dsp/peak_detection.h"
#include "pdt/dsp/spectrum.h"
#include "pdt/dsp/window.h"

#include <cstddef>
#include <span>
#include <vector>

namespace pdt {

// config engine
struct SpectrumAnalysisOptions {
    double sample_rate{};
    WindowType window{WindowType::None};
    PeakDetectionMode peak_mode{PeakDetectionMode::LocalMaxima};
    double threshold{};
    std::size_t top{10};
};

struct SpectrumAnalysisResult {
    Spectrum spectrum;
    std::vector<Peak> all_peaks;
    std::vector<Peak> top_peaks;
    SpectrumAlgorithm algorithm{SpectrumAlgorithm::Auto};
};

class SpectrumEngine {
  public:
    explicit SpectrumEngine(IFftBackend& backend);

    SpectrumAnalysisResult process(std::span<const double> signal, const SpectrumAnalysisOptions& options) const;

  private:
    IFftBackend& backend_;
};

} // namespace pdt
