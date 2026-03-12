#include "pdt/fft.h"
#include "pdt/peak_detection.h"

#include <cmath>
#include <iostream>
#include <numbers>
#include <vector>

int main() {
    using namespace pdt;

    // Signal parameters
    const double fs = 1024.0;    // sampling frequency [Hz]
    const double f0 = 50.0;      // first sinusoid frequency [Hz]
    const double f1 = 120.0;     // second sinusoid frequency [Hz]
    const std::size_t N = 1024;  // number of samples

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
