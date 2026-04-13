#pragma once

#include "pdt/pipeline/wav_analysis_service.h"

#include <optional>
#include <span>
#include <vector>

namespace pdt {

struct AnalysisCacheKey {
    SpectrumAlgorithm algorithm{SpectrumAlgorithm::Fft};
    PeakDetectionMode peak_mode{PeakDetectionMode::LocalMaxima};
    WindowType window{WindowType::Hann};
    std::size_t top_peaks{20};
    std::size_t from{0};
    std::size_t window_size{1024};
    double threshold{0.20};

    bool operator==(const AnalysisCacheKey& other) const = default;
};

class WavAnalysisSession {
  public:
    WavAnalysisSession(std::span<const double> samples, double sample_rate);

    const WavAnalysisResult& analyze(const AnalysisCacheKey& key);
    void clear();

  private:
    std::vector<double> samples_;
    double sample_rate_{};

    std::optional<AnalysisCacheKey> last_key_;
    std::optional<WavAnalysisResult> last_result_;
};

} // namespace pdt
