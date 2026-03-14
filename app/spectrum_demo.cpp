#include "spectrum_cli_args.h"

#include "pdt/fft.h"
#include "pdt/peak_detection.h"
#include "pdt/wav_reader.h"
#include "pdt/window.h"

#include <algorithm>
#include <cstddef>
#include <iostream>
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

    std::cout << "\nInput file   : " << options.input_path << '\n';
    std::cout << "Sample rate  : " << wav->sample_rate << " Hz\n";
    std::cout << "Channels     : " << wav->channels << '\n';
    std::cout << "Samples      : " << wav->samples.size() << '\n';
    std::cout << "From sample  : " << options.from << '\n';
    std::cout << "Bins         : " << segment.size() << '\n';
    std::cout << "Window       : " << (options.use_window ? to_string(options.window) : "none") << '\n';
    std::cout << "Algorithm    : " << to_string(used_algorithm) << '\n';
    std::cout << "Threshold    : " << options.threshold << '\n';
    std::cout << "Peak mode    : " << to_string(options.peak_mode) << '\n';
    std::cout << "Top peaks    : " << options.top << '\n';

    std::cout << "\nDominant peaks\n";
    std::cout << "-------------------------------------\n";

    if (dominant_peaks.empty()) {
        std::cout << "No peaks detected.\n";
        return 0;
    }

    for (std::size_t i = 0; i < dominant_peaks.size(); ++i) {
        const auto& peak = dominant_peaks[i];

        std::cout
            << (i + 1) << ". "
            << "f = " << peak.frequency << " Hz"
            << "    |X| = " << peak.magnitude
            << "    (bin " << peak.index << ")\n";
    }

    return 0;
}
