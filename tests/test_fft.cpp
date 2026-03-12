#include "pdt/dft.h"
#include "pdt/fft.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <complex>
#include <vector>

int main() {
    using namespace pdt;

    // 1. Power-of-two helper
    assert(!is_power_of_two(0));
    assert(is_power_of_two(1));
    assert(is_power_of_two(2));
    assert(!is_power_of_two(3));
    assert(is_power_of_two(8));
    assert(!is_power_of_two(10));

    // 2. FFT should reject non-power-of-two length
    {
        const std::vector<double> signal{1.0, 2.0, 3.0};
        const auto fft = compute_fft(signal);
        assert(fft.empty());
    }

    // 3. Compare FFT against DFT on a small signal
    {
        const std::vector<double> signal{1,2,3,4,5,6,7,8};

        const auto dft = compute_dft(signal);
        const auto fft = compute_fft(signal);

        assert(dft.size() == fft.size());

        for (std::size_t i = 0; i < dft.size(); ++i) {
            assert(std::abs(dft[i].real() - fft[i].real()) < 1e-9);
            assert(std::abs(dft[i].imag() - fft[i].imag()) < 1e-9);
        }
    }

    // 4. FFT spectrum peak for sinusoid
    {
        const double fs = 1024.0;
        const double f0 = 64.0;
        const std::size_t N = 1024;

        std::vector<double> signal;
        signal.reserve(N);

        for (std::size_t n = 0; n < N; ++n) {
            const double sample =
                std::sin(2.0 * std::numbers::pi_v<double> * f0 * static_cast<double>(n) / fs);
            signal.push_back(sample);
        }

        const auto spectrum = compute_single_sided_spectrum_fft(signal, fs);

        assert(!spectrum.frequencies.empty());
        assert(spectrum.frequencies.size() == spectrum.magnitudes.size());

        const auto max_it = std::max_element(
            spectrum.magnitudes.begin(), spectrum.magnitudes.end());

        const auto max_index = static_cast<std::size_t>(
            std::distance(spectrum.magnitudes.begin(), max_it));

        assert(std::abs(spectrum.frequencies[max_index] - f0) < 1e-9);
    }

    return 0;
}
