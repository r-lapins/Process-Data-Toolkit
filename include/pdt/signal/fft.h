#pragma once

#include "dft.h"

#include <complex>
#include <vector>
#include <span>

namespace pdt {

bool is_power_of_two(std::size_t n);

std::vector<std::complex<double>> compute_fft(std::span<const double> signal);

Spectrum compute_single_sided_spectrum_fft(std::span<const double> signal, double sample_rate);

// Automatic selection: FFT for power-of-two sizes, otherwise DFT
Spectrum compute_spectrum(std::span<const double> signal, double sample_rate);

} // namespace pdt

/*
 * Recursive radix-2 Cooley–Tukey FFT.
 *
 * The input sequence is split into even and odd indexed samples:
 *
 * X[k] = FFT(even) + W_N^k * FFT(odd)
 * X[k + N/2] = FFT(even) - W_N^k * FFT(odd)
 *
 * where Twiddle factor: W_N^k = exp(-j*2πk/N)
 *
 * Complexity:
 * O(N log N) compared to O(N²) for the naive DFT.
 *
 * Precondition:
 * signal size must be a power of two.
 */
