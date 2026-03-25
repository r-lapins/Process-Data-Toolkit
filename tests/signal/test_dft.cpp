#include "pdt/signal/dft.h"
#include "pdt/signal/peak_detection.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <vector>

int main() {
    using namespace pdt;

    /*
     * Test parameters
     *
     * Fs = sampling frequency
     * f0 = sinusoid frequency
     * N  = number of samples
     */
    const double fs = 1000.0;
    const double f0 = 50.0;
    const std::size_t N = 1000;

    std::vector<double> signal;
    signal.reserve(N);

    /*
     * Generate a synthetic sinusoidal signal:
     *
     * x[n] = sin(2π f0 n / Fs)
     *
     * This signal should produce a dominant spectral peak at f0.
     */
    for (std::size_t n = 0; n < N; ++n) {
        const double sample = std::sin(2.0 * std::numbers::pi_v<double> * f0 * static_cast<double>(n) / fs);
        signal.push_back(sample);
    }

    // Compute spectrum
    const auto spectrum = compute_single_sided_spectrum(signal, fs);

    // Basic sanity checks
    assert(!spectrum.frequencies.empty());
    assert(spectrum.frequencies.size() == spectrum.magnitudes.size());

    /*
     * Find the maximum magnitude in the spectrum.
     * This should correspond to the sinusoid frequency.
     */
    const auto max_it = std::ranges::max_element(spectrum.magnitudes);

    const auto max_index = static_cast<std::size_t>(std::distance(spectrum.magnitudes.begin(), max_it));

    const double peak_frequency = spectrum.frequencies[max_index];

    /*
     * Verify that the detected peak frequency matches the
     * generated sinusoid frequency.
     *
     * With Fs = 1000 and N = 1000,
     * frequency resolution = 1 Hz,
     * so 50 Hz lands exactly on a spectral bin.
     */
    assert(std::abs(peak_frequency - f0) < 1e-9);

    //
    // Threshold-based peak detection on the computed spectrum
    const auto peaks = find_peaks(spectrum, 0.8, PeakDetectionMode::LocalMaxima);
    assert(!peaks.empty());

    bool found_target_peak = false;
    for (const auto& peak : peaks) {
        if (std::abs(peak.frequency - f0) < 1e-9) {
            found_target_peak = true;
            break;
        }
    }

    assert(found_target_peak);

    return 0;
}
