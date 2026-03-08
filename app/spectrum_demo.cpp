#include "pdt/dft.h"
#include "pdt/peak_detection.h"
#include "pdt/wav_reader.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    using namespace pdt;

    if (argc < 2) {
        std::cerr << "Usage: spectrum_demo <input.wav>\n";
        return 1;
    }

    const auto wav = read_wav_pcm16_mono(argv[1]);
    if (!wav) {
        std::cerr << "Failed to read WAV file or unsupported format.\n";
        return 1;
    }

    std::cout << "Loaded WAV file\n";
    std::cout << "sample_rate = " << wav->sample_rate << " Hz\n";
    std::cout << "channels    = " << wav->channels << "\n";
    std::cout << "samples     = " << wav->samples.size() << "\n";

           // Analyze only a fragment to keep DFT runtime reasonable.
           // DFT is O(N^2), so do not feed the entire file if it is large.
    const std::size_t start = 300;
    const std::size_t end = std::min<std::size_t>(10000, wav->samples.size());

    if (start >= end) {
        std::cerr << "Selected sample range is invalid.\n";
        return 1;
    }

    std::vector<double> segment(wav->samples.begin() + static_cast<std::ptrdiff_t>(start),
                                wav->samples.begin() + static_cast<std::ptrdiff_t>(end));

    std::cout << "Analyzed range: [" << start << ", " << end << ")\n";
    std::cout << "segment size   = " << segment.size() << "\n";

    const auto spectrum = compute_single_sided_spectrum(segment, wav->sample_rate);

    const auto dominant_peaks = detect_dominant_peaks(
        spectrum,
        0.4,
        PeakDetectionMode::LocalMaxima,
        10
        );

    std::cout << "\nDominant peaks\n";
    std::cout << "-------------------------------------\n";

    for (const auto& peak : dominant_peaks) {
        std::cout
            << "index=" << peak.index
            << "  f=" << peak.frequency
            << " Hz"
            << "  |X|=" << peak.magnitude
            << '\n';
    }

    return 0;
}

/*

#include "pdt/dft.h"
#include "pdt/peak_detection.h"

#include <cmath>
#include <iostream>
#include <numbers>
#include <vector>

int main() {
    using namespace pdt;

    // Signal parameters
    const double fs = 1000.0;    // sampling frequency [Hz]
    const double f0 = 50.0;      // first sinusoid frequency [Hz]
    const double f1 = 120.0;     // second sinusoid frequency [Hz]
    const std::size_t N = 1000;  // number of samples

    // Generate a signal containing two sinusoids with different amplitudes
    std::vector<double> signal;
    signal.reserve(N);

    for (std::size_t n = 0; n < N; ++n) {
        const double t = static_cast<double>(n) / fs;

        const double sample =
            std::sin(2.0 * std::numbers::pi_v<double> * f0 * t) +
            (0.5 * std::sin(2.0 * std::numbers::pi_v<double> * f1 * t));

        signal.push_back(sample);
    }

    // Compute single-sided spectrum
    const auto spectrum = compute_single_sided_spectrum(signal, fs);

    std::cout << "Single-sided spectrum\n";
    std::cout << "-------------------------------------\n";

    for (std::size_t i = 0; i < spectrum.frequencies.size(); ++i) {
        std::cout
            << "bin " << i
            << "  f=" << spectrum.frequencies[i]
            << " Hz"
            << "  |X|=" << spectrum.magnitudes[i]
            << '\n';
    }

    // Detect all bins above threshold
    const auto threshold_peaks = find_peaks(
        spectrum.frequencies,
        spectrum.magnitudes,
        0.4,
        PeakDetectionMode::ThresholdOnly
        );

    std::cout << "\nThreshold-only peaks\n";
    std::cout << "-------------------------------------\n";
    for (const auto& peak : threshold_peaks) {
        std::cout
            << "index=" << peak.index
            << "  f=" << peak.frequency
            << " Hz"
            << "  |X|=" << peak.magnitude
            << '\n';
    }

    // Detect local maxima above threshold
    const auto local_maxima_peaks = find_peaks(
        spectrum.frequencies,
        spectrum.magnitudes,
        0.4,
        PeakDetectionMode::LocalMaxima
        );

    std::cout << "\nLocal-maxima peaks\n";
    std::cout << "-------------------------------------\n";
    for (const auto& peak : local_maxima_peaks) {
        std::cout
            << "index=" << peak.index
            << "  f=" << peak.frequency
            << " Hz"
            << "  |X|=" << peak.magnitude
            << '\n';
    }

    // Detect dominant peaks sorted by descending magnitude
    const auto dominant_peaks = detect_dominant_peaks(
        spectrum,
        0.4,
        PeakDetectionMode::LocalMaxima,
        5
        );

    std::cout << "\nDominant peaks\n";
    std::cout << "-------------------------------------\n";
    for (const auto& peak : dominant_peaks) {
        std::cout
            << "index=" << peak.index
            << "  f=" << peak.frequency
            << " Hz"
            << "  |X|=" << peak.magnitude
            << '\n';
    }

    return 0;
}
*/
