#pragma once

#include "pdt/dsp/peak_detection.h"
#include "pdt/dsp/spectrum.h"
#include "pdt/dsp/window.h"

#include <cstddef>
#include <span>
#include <vector>

namespace pdt {

struct WavAnalysisRequest {
    std::span<const double> samples;
    double sample_rate{};

    SpectrumAlgorithm algorithm{SpectrumAlgorithm::Fft};
    PeakDetectionMode peak_mode{PeakDetectionMode::LocalMaxima};
    WindowType window{WindowType::Hann};

    std::size_t top_peaks{20};
    std::size_t from{0};
    std::size_t window_size{1024};
    double threshold{0.20};
};

struct WavAnalysisResult {
    std::vector<double> raw_segment;
    std::vector<double> processed_segment;
    std::vector<Peak> all_peaks;
    std::vector<Peak> dominant_peaks;
    Spectrum spectrum;
    double total_time_ms{};
};

WavAnalysisResult analyze_wav(const WavAnalysisRequest& request);

} // namespace pdt
