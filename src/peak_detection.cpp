#include "pdt/peak_detection.h"

#include <algorithm>

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

    if (threshold_ratio < 0.0) {
        return peaks;
    }

    const auto max_it = std::ranges::max_element(magnitudes);
    const double threshold = (*max_it) * threshold_ratio;

    if (mode == PeakDetectionMode::ThresholdOnly) {
        for (std::size_t i = 0; i < magnitudes.size(); ++i) {
            if (magnitudes[i] >= threshold) {
                peaks.push_back(Peak{.index = i,
                                     .frequency = frequencies[i],
                                     .magnitude = magnitudes[i]
                });
            }
        }

    return peaks;
    }

    if (mode == PeakDetectionMode::LocalMaxima) {
        if (magnitudes.size() < 3) {
            return peaks;
        }

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
    }

    return peaks;
}

} // namespace pdt
