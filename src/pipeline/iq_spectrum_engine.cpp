#include "pdt/pipeline/iq_spectrum_engine.h"

#include <chrono>

namespace pdt {

IqSpectrumEngine::IqSpectrumEngine(IFftBackend& backend)
    : backend_(backend)
{
}

SpectrumAnalysisResult IqSpectrumEngine::process(const IqFrame& frame,
                                           const SpectrumAnalysisOptions& options) const
{
    using clock = std::chrono::steady_clock;

    const auto t0 = clock::now();

    SpectrumAnalysisResult result{};

    const auto fft = backend_.compute_iq_spectrum(frame.samples,
                                                  static_cast<double>(frame.sample_rate),
                                                  options.window);

    result.spectrum = fft.spectrum;
    result.algorithm = fft.algorithm;

    result.all_peaks = find_peaks(result.spectrum,
                                  options.threshold,
                                  options.peak_mode);

    result.top_peaks = select_dominant_peaks(result.all_peaks,
                                             options.max_peaks);

    const auto t1 = clock::now();
    result.total_time_ms =
        std::chrono::duration<double, std::milli>(t1 - t0).count();

    return result;
}

} // namespace pdt
