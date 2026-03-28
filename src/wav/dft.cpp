#include "pdt/wav/dft.h"

#include <cmath>

namespace pdt {

std::vector<std::complex<double>> compute_dft(std::span<const double> signal) {
    // N = number of input samples (DFT size)
    const std::size_t N = signal.size();

    // Result vector of complex spectral coefficients X[k]
    std::vector<std::complex<double>> output(N);

    // Outer loop iterates over frequency bins k (DFT output indices)
    for (std::size_t k = 0; k < N; ++k) {

        // Accumulator for the k-th spectral coefficient
        std::complex<double> sum{0.0, 0.0};

        // Inner loop iterates over time-domain samples
        for (std::size_t n = 0; n < N; ++n) {

            /*
             * Complex exponential (DFT kernel):
             *
             * exp(-j*2*pi*k*n/N)
             *
             * Using Euler identity:
             * exp(jθ) = cos(θ) + j*sin(θ)
             */
            const double angle = -2.0 * std::numbers::pi_v<double> * static_cast<double>(k) * static_cast<double>(n) / static_cast<double>(N);

            // Twiddle factor W_N^(k*n)
            const std::complex<double> twiddle{ std::cos(angle), std::sin(angle) };

            // Accumulate contribution of sample x[n]
            sum += signal[n] * twiddle;
        }

        output[k] = sum;
    }

    return output;
}

Spectrum compute_single_sided_spectrum(std::span<const double> signal, double sample_rate) {

    // Compute full DFT
    const auto dft = compute_dft(signal);

    const std::size_t N = signal.size();

    // For real-valued signals the DFT is symmetric.
    // Only non-negative frequencies (0 .. Fs/2) are kept.
    const std::size_t half = N / 2;

    Spectrum spectrum{};

    spectrum.frequencies.reserve(half + 1);
    spectrum.magnitudes.reserve(half + 1);

    for (std::size_t k = 0; k <= half; ++k) {

        /*
         * Frequency corresponding to bin k:
         *
         * f_k = k * Fs / N
         *
         * where Fs is the sampling rate
         */
        const double frequency = static_cast<double>(k) * sample_rate / static_cast<double>(N);

        // Magnitude of the complex spectral coefficient |X[k]|
        const double magnitude = std::abs(dft[k]);

        spectrum.frequencies.push_back(frequency);
        spectrum.magnitudes.push_back(magnitude);
    }

    return spectrum;
}

} // namespace pdt
