#include "pdt/compute/cpu_fft_backend.h"

#include "pdt/dsp/complex_fft.h"
#include "pdt/dsp/dft.h"
#include "pdt/dsp/fft.h"
#include "pdt/dsp/window.h"

#include <complex>
#include <stdexcept>

namespace pdt {
namespace {

std::vector<std::complex<float>> apply_window_iq(std::span<const std::complex<float>> iq,
                                                 pdt::WindowType window)
{
    if (window == pdt::WindowType::None) {
        return {iq.begin(), iq.end()};
    }

    const auto w = pdt::make_window(window, iq.size());

    std::vector<std::complex<float>> out;
    out.reserve(iq.size());

    for (std::size_t i = 0; i < iq.size(); ++i) {
        out.push_back(iq[i] * static_cast<float>(w[i]));
    }

    return out;
}

} // namespace

FftComputationResult CpuFftBackend::compute_spectrum(std::span<const double> signal,
                                                                  double sample_rate,
                                                                  WindowType window) {
    const auto windowed = apply_window(signal, window);

    if (is_power_of_two(windowed.size())) {
        return {
            .spectrum = compute_single_sided_spectrum_fft(windowed, sample_rate),
            .algorithm = SpectrumAlgorithm::Fft
        };
    }

    return {
        .spectrum = compute_single_sided_spectrum(windowed, sample_rate),
        .algorithm = SpectrumAlgorithm::Dft
    };
}

FftComputationResult CpuFftBackend::compute_iq_spectrum(std::span<const std::complex<float>> iq,
                                                        double sample_rate,
                                                        WindowType window) {
    if (iq.empty() || sample_rate <= 0.0) { return {}; }

    const auto windowed = apply_window_iq(iq, window);
    const std::size_t N = windowed.size();

    if (!is_power_of_two(N)) {
        throw std::invalid_argument("CPU IQ FFT currently requires power-of-two size");
    }

    return {
        .spectrum = compute_centered_iq_spectrum(windowed, sample_rate),
        .algorithm = SpectrumAlgorithm::Fft
    };
}

} // namespace pdt
