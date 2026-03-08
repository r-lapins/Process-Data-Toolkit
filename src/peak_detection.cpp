#include "pdt/peak_detection.h"

#include <algorithm>

namespace pdt {

std::vector<Peak> find_peaks(std::span<const double> frequencies,
                             std::span<const double> magnitudes,
                             double threshold_ratio) {
    std::vector<Peak> peaks{};

    // Invalid input: frequency and magnitude vectors must match
    if (frequencies.size() != magnitudes.size()) {
        return peaks;
    }

    // Empty spectrum -> no peaks
    if (magnitudes.empty()) {
        return peaks;
    }

    // Negative threshold ratio is treated as invalid input
    if (threshold_ratio < 0.0) {
        return peaks;
    }

    const auto max_it = std::ranges::max_element(magnitudes);

    const double threshold = (*max_it) * threshold_ratio;

    for (std::size_t i = 0; i < magnitudes.size(); ++i) {
        if (magnitudes[i] >= threshold) {
            peaks.push_back(Peak{
                .index = i,
                .frequency = frequencies[i],
                .magnitude = magnitudes[i]
            });
        }
    }

    return peaks;
}

} // namespace pdt
