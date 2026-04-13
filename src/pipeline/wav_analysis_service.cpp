#include "pdt/pipeline/wav_analysis_service.h"

#include "pdt/compute/make_fft_backend.h"
#include "pdt/dsp/window.h"
#include "pdt/pipeline/spectrum_engine.h"

#include <algorithm>
#include <stdexcept>

namespace pdt {

WavAnalysisResult analyze_wav(const WavAnalysisRequest& request)
{
    if (request.sample_rate <= 0.0) { throw std::invalid_argument("sample_rate must be > 0"); }

    if (request.from > request.samples.size()) { throw std::out_of_range("from is out of range"); }

    const std::size_t available = request.samples.size() - request.from;
    const std::size_t segment_size = std::min(request.window_size, available);

    if (segment_size == 0) { throw std::invalid_argument("selected segment is empty"); }

    std::vector<double> raw_segment(request.samples.begin() + static_cast<std::ptrdiff_t>(request.from),
                                    request.samples.begin() + static_cast<std::ptrdiff_t>(request.from + segment_size));

    std::vector<double> processed_segment =
        (request.window == WindowType::None) ? raw_segment : apply_window(raw_segment, request.window);

    auto backend = create_fft_backend(request.algorithm);
    SpectrumEngine engine{*backend};

    SpectrumAnalysisOptions options{.sample_rate = request.sample_rate,
                                    .window = request.window,
                                    .peak_mode = request.peak_mode,
                                    .threshold = request.threshold,
                                    .top = request.top_peaks};


    const auto analysis = engine.process(raw_segment, options);

    return WavAnalysisResult{.raw_segment = std::move(raw_segment),
                             .processed_segment = std::move(processed_segment),
                             .all_peaks = analysis.all_peaks,
                             .dominant_peaks = analysis.top_peaks,
                             .spectrum = analysis.spectrum,
                             .total_time_ms = analysis.total_time_ms};
}

} // namespace pdt
