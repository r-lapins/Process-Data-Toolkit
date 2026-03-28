#pragma once

#include "spectrum.h"

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

std::vector<Peak> find_peaks(const Spectrum& spectrum,
                             double threshold_ratio,
                             PeakDetectionMode mode);

std::vector<Peak> select_dominant_peaks(std::span<const Peak> peaks,
                                        std::size_t max_count);

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
 * Detects spectral peaks in a magnitude spectrum using a threshold
 * relative to the maximum magnitude.
 *
 * Threshold:
 *   threshold = threshold_ratio * max(magnitudes)
 *
 * Only bins with magnitude >= threshold are considered.
 *
 * Modes:
 *
 *   ThresholdOnly:
 *     - All bins above the threshold are returned.
 *     - No neighborhood analysis is performed.
 *
 *   LocalMaxima:
 *     - Only bins that are local maxima are returned.
 *     - A bin is a local maximum if:
 *         magnitudes[i] > magnitudes[i-1] &&
 *         magnitudes[i] > magnitudes[i+1]
 *     - Edge bins are ignored (no neighbors).
 *
 * Preconditions:
 *   - frequencies.size() == magnitudes.size()
 *   - magnitudes must not be empty
 *   - threshold_ratio must be in range [0.0, 1.0]
 *
 * Returns:
 *   A vector of Peak structures:
 *     - index      : bin index
 *     - frequency  : corresponding frequency (Hz)
 *     - magnitude  : spectral magnitude
 *
 * Notes:
 *   - The returned peaks are NOT sorted.
 *   - Use select_dominant_peaks(...) to sort and limit results.
 */

/*
 * select_dominant_peaks
 *
 * Selects the strongest peaks from a precomputed list of peaks.
 *
 * Behavior:
 *   - Peaks are sorted in descending order by magnitude.
 *   - The result is truncated to at most max_count elements.
 *
 * Parameters:
 *   peaks      - input list of detected peaks
 *   max_count  - maximum number of peaks to return
 *
 * Returns:
 *   A vector containing up to max_count peaks with the highest magnitude.
 *
 * Notes:
 *   - This function does NOT perform peak detection.
 *   - Use find_peaks(...) before calling this function.
 *   - If peaks is empty or max_count == 0, an empty vector is returned.
 */

/*
 * detect_dominant_peaks
 *
 * Convenience function that combines peak detection and selection
 * of dominant peaks in a single call.
 *
 * Equivalent to:
 *
 *   auto peaks = find_peaks(...);
 *   return select_dominant_peaks(peaks, max_count);
 *
 * Parameters:
 *   spectrum        - input spectrum (frequencies + magnitudes)
 *   threshold_ratio - relative threshold in range [0.0, 1.0]
 *   mode            - peak detection mode
 *   max_count       - maximum number of dominant peaks to return
 *
 * Returns:
 *   A vector of up to max_count strongest peaks.
 *
 * Notes:
 *   - Internally calls find_peaks(...) and select_dominant_peaks(...).
 *   - Prefer using find_peaks(...) + select_dominant_peaks(...)
 *     when peak reuse is needed (e.g. GUI + statistics).
 */
