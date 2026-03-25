#include "pdt/signal/peak_detection.h"

#include <algorithm>

namespace pdt {

std::vector<Peak> find_peaks(const Spectrum& spectrum,
                             double threshold_ratio,
                             PeakDetectionMode mode) {
    std::vector<Peak> peaks{};

    if (spectrum.frequencies.size() != spectrum.magnitudes.size()) { return peaks; }

    if (spectrum.magnitudes.empty()) { return peaks; }

    if (threshold_ratio < 0.0 || threshold_ratio > 1.0) { return peaks; }

    const auto max_it = std::ranges::max_element(spectrum.magnitudes);
    const double threshold = (*max_it) * threshold_ratio;

    using enum PeakDetectionMode;
    switch (mode) {
    case ThresholdOnly:
        for (std::size_t i = 0; i < spectrum.magnitudes.size(); ++i) {
            if (spectrum.magnitudes[i] >= threshold) {
                peaks.push_back(Peak{.index = i,
                                     .frequency = spectrum.frequencies[i],
                                     .magnitude = spectrum.magnitudes[i]
                });
            }
        }
        break;

    case LocalMaxima:
        if (spectrum.magnitudes.size() < 3) { return peaks; }

        // Edge bins are skipped because local maxima detection
        // requires both left and right neighbors.
        for (std::size_t i = 1; i + 1 < spectrum.magnitudes.size(); ++i) {
            const bool above_threshold = spectrum.magnitudes[i] >= threshold;
            const bool is_local_max = spectrum.magnitudes[i] > spectrum.magnitudes[i - 1]
                                      && spectrum.magnitudes[i] > spectrum.magnitudes[i + 1];

            if (above_threshold && is_local_max) {
                peaks.push_back(Peak{.index = i,
                                     .frequency = spectrum.frequencies[i],
                                     .magnitude = spectrum.magnitudes[i]
                });
            }
        }
        break;
    }

    return peaks;
}

std::vector<Peak> select_dominant_peaks(std::span<const Peak> peaks, std::size_t max_count) {
    if (max_count == 0 || peaks.empty()) {
        return {};
    }

    std::vector<Peak> result(peaks.begin(), peaks.end());
    auto k = static_cast<std::ptrdiff_t>(std::min(max_count, result.size()));

    std::ranges::partial_sort(result, result.begin() + k, std::ranges::greater{}, &Peak::magnitude);

    result.resize(k);

    return result;
}

std::vector<Peak> detect_dominant_peaks(const Spectrum& spectrum, double threshold_ratio, PeakDetectionMode mode, std::size_t max_count) {
    const auto peaks = find_peaks(spectrum, threshold_ratio, mode);

    return select_dominant_peaks(peaks, max_count);
}

} // namespace pdt
