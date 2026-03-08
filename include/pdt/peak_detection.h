#pragma once
#include <cstddef>
#include <span>
#include <vector>

namespace pdt {

struct Peak {
    std::size_t index;
    double frequency;
    double magnitude;
};

enum class PeakDetectionMode {
    ThresholdOnly,
    LocalMaxima
};

std::vector<Peak> find_peaks(std::span<const double> frequencies,
                             std::span<const double> magnitudes,
                             double threshold_ratio,
                             PeakDetectionMode mode);

} // namespace pdt

/*
 * Peak
 *
 * Represents one detected spectral peak.
 *
 * index      - bin index in the spectrum
 * frequency  - frequency corresponding to the bin
 * magnitude  - spectral magnitude at that bin
 */

/*
 * find_peaks
 *
 * Detects spectral bins whose magnitude is above a threshold defined
 * as a ratio of the maximum magnitude in the spectrum.
 *
 * Example:
 *   threshold_ratio = 0.8
 * means:
 *   keep all bins with magnitude >= 0.8 * max(magnitudes)
 *
 * Notes:
 *   - This is a simple threshold-based detector.
 *   - It does not yet enforce local maxima detection.
 *   - frequencies and magnitudes must have the same size.
 */
