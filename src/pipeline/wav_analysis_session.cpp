#include "pdt/pipeline/wav_analysis_session.h"

namespace pdt {

WavAnalysisSession::WavAnalysisSession(std::span<const double> samples, double sample_rate)
    : samples_(samples.begin(), samples.end()),
      sample_rate_(sample_rate)
{
}

const WavAnalysisResult& WavAnalysisSession::analyze(const WavAnalysisSettingsCache& key) {
    if (last_key_ && last_result_) {
        if (*last_key_ == key) { return *last_result_; }

        if (can_refresh_top_peaks_only(key)) {
            refresh_top_peaks_only(key);
            last_key_ = key;
            return *last_result_;
        }
    }

    WavAnalysisRequest request{.samples = samples_,
                               .sample_rate = sample_rate_,
                               .algorithm = key.algorithm,
                               .peak_mode = key.peak_mode,
                               .window = key.window,
                               .top_peaks = key.top_peaks,
                               .from = key.from,
                               .window_size = key.window_size,
                               .threshold = key.threshold};

    last_result_ = analyze_wav(request);
    last_key_ = key;

    return *last_result_;
}

void WavAnalysisSession::clear() {
    last_key_.reset();
    last_result_.reset();
}

bool WavAnalysisSession::can_refresh_top_peaks_only(const WavAnalysisSettingsCache& current) const noexcept
{
    if (!last_key_ || !last_result_) { return false; }

    const auto& previous = *last_key_;

    return previous.sample_rate == current.sample_rate &&
           previous.algorithm == current.algorithm &&
           previous.peak_mode == current.peak_mode &&
           previous.window == current.window &&
           previous.from == current.from &&
           previous.window_size == current.window_size &&
           previous.threshold == current.threshold &&
           previous.top_peaks != current.top_peaks;
}

void WavAnalysisSession::refresh_top_peaks_only(const WavAnalysisSettingsCache& current)
{
    if (!last_result_) {
        return;
    }

    last_result_->analysis.top_peaks =
        select_dominant_peaks(last_result_->analysis.all_peaks, current.top_peaks);

    last_result_->used_settings.top_peaks = current.top_peaks;
}

} // namespace pdt
