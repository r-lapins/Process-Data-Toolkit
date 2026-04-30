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

    const auto& settings = request.settings;

    if (settings.from > request.samples.size()) { throw std::out_of_range("from is out of range"); }

    const std::size_t available = request.samples.size() - settings.from;
    const std::size_t segment_size = std::min(settings.window_size, available);

    if (segment_size == 0) { throw std::invalid_argument("selected segment is empty"); }

    std::vector<double> raw_segment(request.samples.begin() + static_cast<std::ptrdiff_t>(settings.from),
                                    request.samples.begin() + static_cast<std::ptrdiff_t>(settings.from + segment_size));

    std::vector<double> processed_segment =
        (settings.window == WindowType::None) ? raw_segment : apply_window(raw_segment, settings.window);

    auto backend = create_fft_backend(settings.algorithm);
    SpectrumEngine engine{*backend};

    SpectrumAnalysisOptions options{.sample_rate = request.sample_rate,
                                    .window = settings.window,
                                    .peak_mode = settings.peak_mode,
                                    .threshold = settings.threshold,
                                    .max_peaks = settings.max_peaks};


    const auto analysis = engine.process(raw_segment, options);

    return WavAnalysisResult{.analysis = analysis,
                             .raw_segment = std::move(raw_segment),
                             .processed_segment = std::move(processed_segment),
                             .used_settings = settings};
}

} // namespace pdt
