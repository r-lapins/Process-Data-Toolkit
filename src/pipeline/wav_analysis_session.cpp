#include "pdt/pipeline/wav_analysis_session.h"

namespace pdt {

WavAnalysisSession::WavAnalysisSession(std::span<const double> samples, double sample_rate)
    : samples_(samples.begin(), samples.end()),
      sample_rate_(sample_rate)
{
}

const WavAnalysisResult& WavAnalysisSession::analyze(const AnalysisCacheKey& key) {
    if (last_key_ && last_result_ && *last_key_ == key) { return *last_result_; }

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

} // namespace pdt
