#include "spectrum_cli_args.h"

#include "pdt/signal/dft.h"
#include "pdt/signal/fft.h"
#include "pdt/signal/window.h"
#include "pdt/signal/wav_reader.h"
#include "pdt/signal/peak_detection.h"
#include "pdt/signal/spectrum_report.h"

#include <algorithm>
#include <iostream>
#include <cstddef>
#include <vector>

int main(int argc, char* argv[]) {
    using namespace pdt;
    using namespace spectrum_app;

    CliOptions options{};

    if (!parse_cli(argc, const_cast<const char* const*>(argv), options, std::cerr)) {
        print_help(std::cerr);
        return 1;
    }

    if (options.help_requested) {
        print_help(std::cout);
        return 0;
    }

    const auto wav = read_wav_pcm16_mono(options.input_path.c_str());
    if (!wav.has_value()) {
        std::cerr << "Failed to read WAV file or unsupported format.\n";
        return 1;
    }

    if (options.from >= wav->samples.size()) {
        std::cerr << "Start sample is out of range.\n";
        return 1;
    }

    if (options.from + options.bins > wav->samples.size()) {
        std::cerr << "Last " << options.from + options.bins - wav->samples.size() << " samples are out of range.\n";
        return 1;
    }

    const std::size_t available = wav->samples.size() - options.from;
    const std::size_t segment_size = std::min(options.bins, available);

    if (segment_size == 0) {
        std::cerr << "Selected segment is empty.\n";
        return 1;
    }

    std::vector<double> segment(
        wav->samples.begin() + static_cast<std::ptrdiff_t>(options.from),
        wav->samples.begin() + static_cast<std::ptrdiff_t>(options.from + segment_size)
        );

    if (options.use_window) {
        segment = apply_window(segment, options.window);
    }

    Spectrum spectrum{};
    SpectrumAlgorithm used_algorithm = options.algorithm;

    switch (options.algorithm) {
    case SpectrumAlgorithm::Auto:
        if (is_power_of_two(segment.size())) {
            spectrum = compute_single_sided_spectrum_fft(segment, wav->sample_rate);
            used_algorithm = SpectrumAlgorithm::Fft;
        } else {
            spectrum = compute_single_sided_spectrum(segment, wav->sample_rate);
            used_algorithm = SpectrumAlgorithm::Dft;
        }
        break;

    case SpectrumAlgorithm::Dft:
        spectrum = compute_single_sided_spectrum(segment, wav->sample_rate);
        break;

    case SpectrumAlgorithm::Fft:
        if (!is_power_of_two(segment.size())) {
            std::cerr << "FFT requires number of bins to be a power of two.\n";
            return 1;
        }
        spectrum = compute_single_sided_spectrum_fft(segment, wav->sample_rate);
        break;
    }

    const auto dominant_peaks = detect_dominant_peaks(
        spectrum,
        options.threshold,
        options.peak_mode,
        options.top
        );

    pdt::SpectrumReport report{
        .spectrum = spectrum,
        .peaks = dominant_peaks,
        .meta = {
            .input_path = options.input_path,
            .sample_rate = static_cast<double>(wav->sample_rate),
            .channels = wav->channels,
            .total_samples = wav->samples.size(),
            .from = options.from,
            .bins = segment.size(),
            .window = options.use_window ? to_string(options.window) : "none",
            .algorithm = to_string(used_algorithm),
            .threshold = options.threshold,
            .peak_mode = to_string(options.peak_mode),
            .top = options.top
        }
    };

    std::cout << pdt::format_spectrum_report(report);

    if (!options.output_csv_path.empty()) {
        if (!pdt::export_spectrum_csv(spectrum, options.output_csv_path)) {
            std::cerr << "Failed to export CSV: "
                      << options.output_csv_path << '\n';
            return 1;
        }

        std::cout << "Spectrum exported to: "
                  << options.output_csv_path << '\n';
    }

    return 0;
}
