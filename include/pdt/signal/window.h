#pragma once

#include <cstddef>
#include <vector>
#include <span>

namespace pdt {

enum class WindowType {
    Hann,
    Hamming
};

std::vector<double> make_window(WindowType type, std::size_t size);
std::vector<double> apply_window(std::span<const double> signal, WindowType type);

} // namespace pdt

/*
 *       make_window
 *
 * Returns window coefficients of length N.
 *
 * Hann:
 *   w[n] = 0.5 * (1 - cos(2*pi*n/(N-1)))
 *
 * Hamming:
 *   w[n] = 0.54 - 0.46 * cos(2*pi*n/(N-1))
 */

/*
 *       apply_window
 *
 * Applies a window to a signal sample-by-sample:
 *   y[n] = x[n] * w[n]
 *
 * Returns an empty vector if the window size does not match
 * the signal size.
 */
