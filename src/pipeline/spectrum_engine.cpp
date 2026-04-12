#include "pdt/pipeline/spectrum_engine.h"

namespace pdt {

SpectrumEngine::SpectrumEngine(IFftBackend& backend) : backend_(backend) {}

SpectrumAnalysisResult SpectrumEngine::process(std::span<const double> signal, const SpectrumAnalysisOptions& options) const {
    SpectrumAnalysisResult result{};

    auto fft_result= backend_.compute_spectrum(signal,
                                                options.sample_rate,
                                                options.window);

    result.spectrum = std::move(fft_result.spectrum);
    result.algorithm = fft_result.algorithm;

    result.all_peaks = find_peaks(result.spectrum,
                                  options.threshold,
                                  options.peak_mode);

    result.top_peaks = select_dominant_peaks(result.all_peaks,
                                             options.top);

    return result;
}

} // namespace pdt
