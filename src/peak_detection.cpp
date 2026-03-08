#include "pdt/peak_detection.h"

#include <algorithm>
#include <ranges>

namespace pdt {

std::vector<Peak> find_peaks(std::span<const double> frequencies,
                             std::span<const double> magnitudes,
                             double threshold_ratio,
                             PeakDetectionMode mode) {
    std::vector<Peak> peaks{};

    // Invalid input: frequency and magnitude vectors must match
    if (frequencies.size() != magnitudes.size()) {
        return peaks;
    }

    if (magnitudes.empty()) {
        return peaks;
    }

    if (threshold_ratio < 0.0 || threshold_ratio > 1.0) {
        return peaks;
    }

    const auto max_it = std::ranges::max_element(magnitudes);
    const double threshold = (*max_it) * threshold_ratio;

    switch (mode) {
    case PeakDetectionMode::ThresholdOnly:

        for (std::size_t i = 0; i < magnitudes.size(); ++i) {
            if (magnitudes[i] >= threshold) {
                peaks.push_back(Peak{.index = i,
                                     .frequency = frequencies[i],
                                     .magnitude = magnitudes[i]
                });
            }
        }
        break;

    case PeakDetectionMode::LocalMaxima:
        if (magnitudes.size() < 3) {
            return peaks;
        }

        // Edge bins are skipped because local maxima detection
        // requires both left and right neighbors.
        for (std::size_t i = 1; i + 1 < magnitudes.size(); ++i) {
            const bool above_threshold = magnitudes[i] >= threshold;
            const bool is_local_max =
                magnitudes[i] > magnitudes[i - 1] &&
                magnitudes[i] > magnitudes[i + 1];

            if (above_threshold && is_local_max) {
                peaks.push_back(Peak{.index = i,
                                     .frequency = frequencies[i],
                                     .magnitude = magnitudes[i]
                });
            }
        }
        break;
    }

    return peaks;
}

std::vector<Peak> detect_dominant_peaks(const Spectrum& spectrum,
                                        double threshold_ratio,
                                        PeakDetectionMode mode,
                                        std::size_t max_count) {
    std::vector<Peak> result{};

    if (max_count == 0) {
        return result;
    }

    auto peaks = find_peaks(spectrum.frequencies, spectrum.magnitudes,
                            threshold_ratio, mode);

    if (peaks.empty()) {
        return result;
    }

    // Sort peaks by descending magnitude
    std::ranges::sort(peaks, [](const Peak& a, const Peak& b) { return a.magnitude > b.magnitude; });

    const std::size_t count = std::min(max_count, peaks.size());
    std::ranges::copy(peaks | std::views::take(count), std::back_inserter(result));
    return result;
}

} // namespace pdt
