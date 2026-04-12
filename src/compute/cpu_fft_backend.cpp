#include "pdt/compute/cpu_fft_backend.h"

#include "pdt/dsp/fft.h"
#include "pdt/dsp/dft.h"
#include "pdt/dsp/window.h"

namespace pdt {

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

} // namespace pdt
