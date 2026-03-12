#include "pdt/fft.h"

// #include <iostream>
#include <numbers>
#include <cmath>

namespace pdt {
namespace {

std::vector<std::complex<double>> compute_fft_recursive(
    std::span<const std::complex<double>> signal) {
    const std::size_t N = signal.size();

    if (N == 1) {
        return {signal[0]};
    }

    std::vector<std::complex<double>> even;
    std::vector<std::complex<double>> odd;
    even.reserve(N / 2);
    odd.reserve(N / 2);

    for (std::size_t i = 0; i < N; ++i) {
        if (i % 2 == 0) {
            even.push_back(signal[i]);
        } else {
            odd.push_back(signal[i]);
        }
    }

    const auto fft_even = compute_fft_recursive(even);
    const auto fft_odd = compute_fft_recursive(odd);

    std::vector<std::complex<double>> result(N);

    for (std::size_t k = 0; k < N / 2; ++k) {
        const double angle =
            - 2.0 * std::numbers::pi_v<double> * static_cast<double>(k) / static_cast<double>(N);

        const std::complex<double> twiddle{std::cos(angle), std::sin(angle)};
        const auto t = twiddle * fft_odd[k];

        result[k] = fft_even[k] + t;
        result[k + (N / 2)] = fft_even[k] - t;
    }

    return result;
}

} // namespace

bool is_power_of_two(std::size_t n)
{
    return n > 0 && (n & (n - 1)) == 0; // & - bitwise 'and'; A power of two has exactly one bit set
}

std::vector<std::complex<double> > compute_fft(std::span<const double> signal)
{
    if (!is_power_of_two(signal.size())) {
        return {};
    }

    std::vector<std::complex<double>> complex_signal;
    complex_signal.reserve(signal.size());

    for (double sample : signal) {
        complex_signal.emplace_back(sample, 0.0);
    }

    return compute_fft_recursive(complex_signal);
}

Spectrum compute_single_sided_spectrum_fft(std::span<const double> signal, double sample_rate)
{
    // For real-valued signals the spectrum is symmetric.
    // Only the positive frequency half is returned.
    Spectrum spectrum{};

    if (signal.empty() || sample_rate <= 0.0) {
        return spectrum;
    }

    const auto fft = compute_fft(signal);
    if (fft.empty()) {
        return spectrum;
    }

    const std::size_t N = signal.size();
    const std::size_t half = N / 2;

    spectrum.frequencies.reserve(half + 1);
    spectrum.magnitudes.reserve(half + 1);

    for (std::size_t k = 0; k <= half; ++k) {
        // Frequency corresponding to bin k: f_k = k * Fs / N
        const double frequency = static_cast<double>(k) * sample_rate / static_cast<double>(N);
        const double magnitude = std::abs(fft[k]);

        spectrum.frequencies.push_back(frequency);
        spectrum.magnitudes.push_back(magnitude);
    }

    return spectrum;
}

Spectrum compute_spectrum(std::span<const double> signal, double sample_rate)
{
    // Automatic selection: FFT for power-of-two sizes, otherwise DFT
    if (is_power_of_two(signal.size())) {
        // std::cout << "Using FFT\n";
        return compute_single_sided_spectrum_fft(signal, sample_rate);
    }

    // std::cout << "Using DFT\n";
    return compute_single_sided_spectrum(signal, sample_rate);
}

} // namespace pdt
