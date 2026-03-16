#pragma once

#include "dft.h"

#include <cstddef>
#include <vector>
#include <span>

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

std::vector<Peak> detect_dominant_peaks(const Spectrum& spectrum,
                                              double threshold_ratio,
                                              PeakDetectionMode mode,
                                              std::size_t max_count
                                              );

// double spectral_centroid(const Spectrum& spectrum);
// double spectral_rolloff(const Spectrum& spectrum, double ratio);
// double spectral_bandwidth(const Spectrum& spectrum);

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

/*
 * detect_dominant_frequencies
 *
 * Returns dominant frequencies detected in the spectrum.
 *
 * Peaks are filtered using the selected detection mode and threshold,
 * then sorted by descending magnitude.
 */
