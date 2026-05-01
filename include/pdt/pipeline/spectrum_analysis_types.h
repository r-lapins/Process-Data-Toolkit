#pragma once

#include "pdt/dsp/peak_detection.h"
#include "pdt/dsp/spectrum.h"
#include "pdt/dsp/window.h"

#include <cstddef>
#include <vector>

namespace pdt {

// config engine
struct SpectrumAnalysisOptions {
    double sample_rate{};
    WindowType window{WindowType::None};
    PeakDetectionMode peak_mode{PeakDetectionMode::LocalMaxima};
    double threshold{};
    std::size_t max_peaks{10};
};

struct SpectrumAnalysisResult {
    Spectrum spectrum;
    std::vector<Peak> all_peaks;
    std::vector<Peak> top_peaks;
    SpectrumAlgorithm algorithm{SpectrumAlgorithm::Auto};
    double total_time_ms{};
};

} // namespace pdt
