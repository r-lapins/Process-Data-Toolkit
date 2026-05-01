#pragma once

#include "pdt/dsp/spectrum.h"

#include <complex>
#include <span>
#include <vector>

namespace pdt {

std::vector<std::complex<float>> compute_fft_complex(std::span<const std::complex<float>> signal);

Spectrum compute_centered_iq_spectrum(std::span<const std::complex<float>> iq,
                                      double sample_rate);

} // namespace pdt
