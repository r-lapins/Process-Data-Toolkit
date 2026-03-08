#pragma once
#include <complex>
#include <span>
#include <vector>

namespace pdt {

struct Spectrum {
    std::vector<double> frequencies;
    std::vector<double> magnitudes;
};

std::vector<std::complex<double>> compute_dft(std::span<const double> signal);

Spectrum compute_single_sided_spectrum(std::span<const double> signal, double sample_rate);

} // namespace pdt


/*
 * Spectrum
 *
 * Holds results of a single-sided amplitude spectrum.
 *
 * frequencies[k]  -> frequency corresponding to bin k
 * magnitudes[k]   -> magnitude |X[k]| of the spectrum
 *
 * Only the positive half of the spectrum is stored
 * (0 .. Fs/2), which is standard for real-valued signals.
 */


/*
 * compute_dft
 *
 * Computes the Discrete Fourier Transform (DFT) of a real-valued signal.
 *
 * Mathematical definition:
 *
 *   X[k] = sum_{n=0}^{N-1} x[n] * exp(-j*2*pi*k*n/N)
 *
 * Complexity:
 *   O(N^2)
 *
 * Notes:
 *   - Implemented intentionally as a simple reference implementation.
 *   - For large signals this should be replaced by FFT.
 */

/*
 * compute_single_sided_spectrum
 *
 * Computes the magnitude spectrum for a real-valued signal and returns
 * only the positive frequency range (0 .. Fs/2).
 *
 * Output:
 *   frequencies[k] = k * Fs / N
 *   magnitudes[k]  = |X[k]|
 *
 * This is equivalent to MATLAB:
 *   X = abs(fft(x));
 *   X = X(1:N/2+1);
 */
