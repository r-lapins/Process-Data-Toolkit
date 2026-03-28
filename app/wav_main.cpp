#include "wav_cli_args.h"

#include "pdt/wav/dft.h"
#include "pdt/wav/fft.h"
#include "pdt/wav/peak_detection.h"
#include "pdt/wav/spectrum.h"
#include "pdt/wav/spectrum_output.h"
#include "pdt/wav/wav_reader.h"
#include "pdt/wav/window.h"

#include <algorithm>
#include <iostream>
#include <fstream>
#include <cstddef>
#include <vector>

int main(int argc, char* argv[]) {
    using namespace pdt;
    using namespace wav_app;

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

    if (options.from + options.windowSize > wav->samples.size()) {
        std::cerr << "Last " << options.from + options.windowSize - wav->samples.size() << " samples are out of range.\n";
        return 1;
    }

    const std::size_t available = wav->samples.size() - options.from;
    const std::size_t segment_size = std::min(options.windowSize, available);

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

    using enum SpectrumAlgorithm;
    switch (options.algorithm) {
    case Auto:
        if (is_power_of_two(segment.size())) {
            spectrum = compute_single_sided_spectrum_fft(segment, wav->sample_rate);
            used_algorithm = Fft;
        } else {
            spectrum = compute_single_sided_spectrum(segment, wav->sample_rate);
            used_algorithm = Dft;
        }
        break;

    case Dft:
        spectrum = compute_single_sided_spectrum(segment, wav->sample_rate);
        break;

    case Fft:
        if (!is_power_of_two(segment.size())) {
            std::cerr << "FFT requires number of bins to be a power of two.\n";
            return 1;
        }
        spectrum = compute_single_sided_spectrum_fft(segment, wav->sample_rate);
        break;
    }

    const auto all_peaks = find_peaks(spectrum, options.threshold, options.peak_mode);
    const auto dominant_peaks = select_dominant_peaks(all_peaks, options.top);

    pdt::SpectrumReport report{
        .spectrum = spectrum,
        .all_peaks = all_peaks,
        .top_peaks = dominant_peaks,
        .meta = {
            .input_path = options.input_path,
            .sample_rate = static_cast<double>(wav->sample_rate),
            .channels = wav->channels,
            .total_samples = wav->samples.size(),
            .from = options.from,
            .windowSize = segment.size(),
            .window = options.use_window ? to_string(options.window) : "none",
            .algorithm = to_string(used_algorithm),
            .threshold = options.threshold,
            .peak_mode = to_string(options.peak_mode),
            .top = options.top
        }
    };

    if (!pdt::write_spectrum_report(std::cout, report)) {
        std::cerr << "Failed to write spectrum report.\n";
        return 1;
    }

    if (!options.output_csv_path.empty()) {
        std::ofstream out_file(options.output_csv_path);
        if (!out_file) {
            std::cerr << "Failed to export CSV: " << options.output_csv_path << '\n';
            return 1;
        }

        if (!write_spectrum_csv(out_file, spectrum)) {
            std::cerr << "Failed to write spectrum CSV.\n";
            return 1;
        }

        std::cout << "Spectrum exported to: " << options.output_csv_path << '\n';
    }

    if (!options.output_report_path.empty()) {
        std::ofstream report_file(options.output_report_path);
        if (!report_file) {
            std::cerr << "Failed to open report file: " << options.output_report_path << '\n';
            return 1;
        }

        if (!write_spectrum_report(report_file, report)) {
            std::cerr << "Failed to write spectrum report.\n";
            return 1;
        }

        std::cout << "Report exported to: " << options.output_report_path << '\n';
    }

    return 0;
}
