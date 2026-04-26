#pragma once

#include "pdt/pipeline/wav_analysis_service.h"

#include <optional>
#include <span>
#include <vector>

namespace pdt {

class WavAnalysisSession {
  public:
    WavAnalysisSession(std::span<const double> samples, double sample_rate);

    const WavAnalysisResult& analyze(const WavAnalysisSettingsCache& key);
    void clear();

  private:
    [[nodiscard]] bool can_refresh_top_peaks_only(const WavAnalysisSettingsCache& current) const noexcept;
    void refresh_top_peaks_only(const WavAnalysisSettingsCache& current);

    std::vector<double> samples_;
    double sample_rate_{};

    std::optional<WavAnalysisSettingsCache> last_key_;
    std::optional<WavAnalysisResult> last_result_;
};

} // namespace pdt
