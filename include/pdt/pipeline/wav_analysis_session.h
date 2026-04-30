#pragma once

#include "pdt/pipeline/wav_analysis_service.h"

#include <optional>
#include <span>
#include <vector>

namespace pdt {

class WavAnalysisSession {
  public:
    WavAnalysisSession(std::span<const double> samples, double sample_rate);

    const WavAnalysisResult& analyze(const WavAnalysisSettings& settings);
    void clear();

  private:
    [[nodiscard]] bool can_refresh_top_peaks_only(const WavAnalysisSettings& current) const noexcept;
    void refresh_top_peaks_only(const WavAnalysisSettings& current);

    std::vector<double> samples_;
    double sample_rate_{};

    std::optional<WavAnalysisSettings> last_settings_;
    std::optional<WavAnalysisResult> last_result_;
};

} // namespace pdt
