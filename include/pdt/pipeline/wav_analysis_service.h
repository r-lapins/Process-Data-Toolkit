#pragma once

#include "pdt/dsp/peak_detection.h"
#include "pdt/dsp/spectrum.h"
#include "pdt/dsp/window.h"
#include "pdt/pipeline/spectrum_engine.h"

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

struct WavAnalysisSettingsCache {
    double sample_rate{};

    SpectrumAlgorithm algorithm{SpectrumAlgorithm::Fft};
    PeakDetectionMode peak_mode{PeakDetectionMode::LocalMaxima};
    WindowType window{WindowType::Hann};

    std::size_t top_peaks{20};
    std::size_t from{0};
    std::size_t window_size{1024};
    double threshold{0.20};

    bool operator==(const WavAnalysisSettingsCache& other) const = default;
};

struct WavAnalysisResult {
    SpectrumAnalysisResult analysis;
    std::vector<double> raw_segment;
    std::vector<double> processed_segment;
    WavAnalysisSettingsCache used_settings;
};

WavAnalysisResult analyze_wav(const WavAnalysisRequest& request);

} // namespace pdt
