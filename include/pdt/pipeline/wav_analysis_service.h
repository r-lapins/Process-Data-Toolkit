#pragma once

#include "pdt/dsp/peak_detection.h"
#include "pdt/dsp/spectrum.h"
#include "pdt/dsp/window.h"
#include "pdt/pipeline/spectrum_engine.h"

#include <cstddef>
#include <span>
#include <vector>

namespace pdt {

struct WavAnalysisSettings {
    SpectrumAlgorithm algorithm{SpectrumAlgorithm::Fft};
    PeakDetectionMode peak_mode{PeakDetectionMode::LocalMaxima};
    WindowType window{WindowType::Hann};

    std::size_t max_peaks{10};
    std::size_t from{0};
    std::size_t window_size{1024};
    double threshold{0.20};

    bool operator==(const WavAnalysisSettings& other) const = default;
};

struct WavAnalysisRequest {
    std::span<const double> samples;
    double sample_rate{};
    WavAnalysisSettings settings;
};

struct WavAnalysisResult {
    SpectrumAnalysisResult analysis;
    std::vector<double> raw_segment;
    std::vector<double> processed_segment;
    WavAnalysisSettings used_settings;
};

WavAnalysisResult analyze_wav(const WavAnalysisRequest& request);

} // namespace pdt
