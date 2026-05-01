#include "pdt/dsp/complex_fft.h"

#include "pdt/dsp/fft.h"

#include <cmath>
#include <numbers>

namespace pdt {
namespace {

std::vector<std::complex<float>> fft_recursive(std::span<const std::complex<float>> signal)
{
    const std::size_t n = signal.size();

    if (n == 1) {
        return {signal[0]};
    }

    std::vector<std::complex<float>> even;
    std::vector<std::complex<float>> odd;

    even.reserve(n / 2);
    odd.reserve(n / 2);

    for (std::size_t i = 0; i < n; ++i) {
        if ((i % 2) == 0) {
            even.push_back(signal[i]);
        } else {
            odd.push_back(signal[i]);
        }
    }

    const auto fft_even = fft_recursive(even);
    const auto fft_odd = fft_recursive(odd);

    std::vector<std::complex<float>> out(n);

    for (std::size_t k = 0; k < n / 2; ++k) {
        const auto angle =
            static_cast<float>(-2.0 * std::numbers::pi_v<double> * static_cast<double>(k) / static_cast<double>(n));

        const std::complex<float> twiddle{
            std::cos(angle),
            std::sin(angle)
        };

        const auto t = twiddle * fft_odd[k];

        out[k] = fft_even[k] + t;
        out[k + (n / 2)] = fft_even[k] - t;
    }

    return out;
}

} // namespace

std::vector<std::complex<float>> compute_fft_complex(std::span<const std::complex<float>> signal)
{
    if (!is_power_of_two(signal.size())) {
        return {};
    }

    if (signal.empty()) {
        return {};
    }

    return fft_recursive(signal);
}

Spectrum compute_centered_iq_spectrum(std::span<const std::complex<float>> iq,
                                      double sample_rate)
{
    Spectrum spectrum{};

    if (iq.empty() || sample_rate <= 0.0) {
        return spectrum;
    }

    const auto fft = compute_fft_complex(iq);
    if (fft.empty()) {
        return spectrum;
    }

    const std::size_t n = fft.size();
    const std::size_t half = n / 2;

    spectrum.frequencies.reserve(n);
    spectrum.magnitudes.reserve(n);

    for (std::size_t i = 0; i < n; ++i) {
        const std::size_t bin = (i + half) % n;

        const double frequency =
            (static_cast<double>(i) - static_cast<double>(half)) * sample_rate / static_cast<double>(n);

        const auto x = fft[bin];
        const double magnitude = std::abs(x);

        spectrum.frequencies.push_back(frequency);
        spectrum.magnitudes.push_back(magnitude);
    }

    return spectrum;
}

} // namespace pdt
