#include "pdt/pipeline/wav_analysis_session.h"

namespace pdt {

WavAnalysisSession::WavAnalysisSession(std::span<const double> samples, double sample_rate)
    : samples_(samples.begin(), samples.end()),
      sample_rate_(sample_rate)
{
}

const WavAnalysisResult& WavAnalysisSession::analyze(const WavAnalysisSettings& settings) {
    if (last_settings_ && last_result_) {
        if (*last_settings_ == settings) { return *last_result_; }

        if (can_refresh_top_peaks_only(settings)) {
            refresh_top_peaks_only(settings);
            last_settings_ = settings;
            return *last_result_;
        }
    }

    WavAnalysisRequest request{.samples = samples_,
                               .sample_rate = sample_rate_,
                               .settings = settings};

    last_result_ = analyze_wav(request);
    last_settings_ = settings;

    return *last_result_;
}

void WavAnalysisSession::clear() {
    last_settings_.reset();
    last_result_.reset();
}

bool WavAnalysisSession::can_refresh_top_peaks_only(const WavAnalysisSettings& current) const noexcept
{
    if (!last_settings_ || !last_result_) { return false; }

    const auto& previous = *last_settings_;

    return previous.algorithm == current.algorithm &&
           previous.peak_mode == current.peak_mode &&
           previous.window == current.window &&
           previous.from == current.from &&
           previous.window_size == current.window_size &&
           previous.threshold == current.threshold &&
           previous.max_peaks != current.max_peaks;
}

void WavAnalysisSession::refresh_top_peaks_only(const WavAnalysisSettings& current)
{
    if (!last_result_) {
        return;
    }

    last_result_->analysis.top_peaks =
        select_dominant_peaks(last_result_->analysis.all_peaks, current.max_peaks);

    last_result_->used_settings.max_peaks = current.max_peaks;
}

} // namespace pdt
