#include "pdt/dsp/fft.h"
#include "pdt/dsp/peak_detection.h"

#include <cmath>
#include <iostream>
#include <numbers>
#include <vector>

int main() {
    using namespace pdt;

    // Signal parameters
    const double fs = 48000.0;
    const double f1 = 1000.0;
    const double f2 = 3500.0;
    const double f3 = 7200.0;
    const double f4 = 15000.0;
    const std::size_t N = 8192;  // number of samples

    // Generate a signal containing two sinusoids with different amplitudes
    std::vector<double> signal;
    signal.reserve(N);

    for (std::size_t n = 0; n < N; ++n) {
        const double t = static_cast<double>(n) / fs;
        const double sample =
            (1.0 * std::sin(2.0 * std::numbers::pi_v<double> * f1 * t)) +
            (0.7 * std::sin(2.0 * std::numbers::pi_v<double> * f2 * t)) +
            (0.5 * std::sin(2.0 * std::numbers::pi_v<double> * f3 * t)) +
            (0.3 * std::sin(2.0 * std::numbers::pi_v<double> * f4 * t));
        signal.push_back(sample);
    }

    // Compute single-sided spectrum
    const auto spectrum = compute_spectrum(signal, fs);

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
    const auto threshold_peaks = find_peaks(spectrum,
                                            0.4,
                                            PeakDetectionMode::ThresholdOnly);

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
    const auto local_maxima_peaks = find_peaks(spectrum,
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
    const auto dominant_peaks = detect_dominant_peaks(spectrum,
                                                      0.4,
                                                      PeakDetectionMode::LocalMaxima,
                                                      5);

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
