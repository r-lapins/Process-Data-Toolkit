#include "pdt/peak_detection.h"

#include <cassert>
#include <vector>

namespace {

using namespace pdt;

void test_find_peaks_empty_input() {
    const std::vector<double> frequencies{};
    const std::vector<double> magnitudes{};

    const auto peaks = find_peaks(
        frequencies, magnitudes, 0.8, PeakDetectionMode::ThresholdOnly);

    assert(peaks.empty());
}

void test_find_peaks_size_mismatch() {
    const std::vector<double> frequencies{0.0, 10.0, 20.0};
    const std::vector<double> magnitudes{1.0, 2.0};

    const auto peaks = find_peaks(
        frequencies, magnitudes, 0.8, PeakDetectionMode::ThresholdOnly);

    assert(peaks.empty());
}

void test_find_peaks_invalid_negative_threshold() {
    const std::vector<double> frequencies{0.0, 10.0, 20.0};
    const std::vector<double> magnitudes{1.0, 2.0, 3.0};

    const auto peaks = find_peaks(
        frequencies, magnitudes, -0.1, PeakDetectionMode::ThresholdOnly);

    assert(peaks.empty());
}

void test_find_peaks_invalid_threshold_above_one() {
    const std::vector<double> frequencies{0.0, 10.0, 20.0};
    const std::vector<double> magnitudes{1.0, 2.0, 3.0};

    const auto peaks = find_peaks(
        frequencies, magnitudes, 1.1, PeakDetectionMode::ThresholdOnly);

    assert(peaks.empty());
}

void test_find_peaks_threshold_only_mode() {
    const std::vector<double> frequencies{0.0, 10.0, 20.0, 30.0, 40.0};
    const std::vector<double> magnitudes{1.0, 8.5, 10.0, 8.7, 2.0};

    const auto peaks = find_peaks(
        frequencies, magnitudes, 0.8, PeakDetectionMode::ThresholdOnly);

    assert(peaks.size() == 3);

    assert(peaks[0].index == 1);
    assert(peaks[0].frequency == 10.0);
    assert(peaks[0].magnitude == 8.5);

    assert(peaks[1].index == 2);
    assert(peaks[1].frequency == 20.0);
    assert(peaks[1].magnitude == 10.0);

    assert(peaks[2].index == 3);
    assert(peaks[2].frequency == 30.0);
    assert(peaks[2].magnitude == 8.7);
}

void test_find_peaks_local_maxima_mode() {
    const std::vector<double> frequencies{0.0, 10.0, 20.0, 30.0, 40.0};
    const std::vector<double> magnitudes{1.0, 8.5, 10.0, 8.7, 2.0};

    const auto peaks = find_peaks(
        frequencies, magnitudes, 0.8, PeakDetectionMode::LocalMaxima);

    assert(peaks.size() == 1);
    assert(peaks[0].index == 2);
    assert(peaks[0].frequency == 20.0);
    assert(peaks[0].magnitude == 10.0);
}

void test_find_peaks_local_maxima_too_few_samples() {
    const std::vector<double> frequencies{0.0, 10.0};
    const std::vector<double> magnitudes{1.0, 2.0};

    const auto peaks = find_peaks(
        frequencies, magnitudes, 0.5, PeakDetectionMode::LocalMaxima);

    assert(peaks.empty());
}

void test_find_peaks_local_maxima_ignores_edges() {
    const std::vector<double> frequencies{0.0, 10.0, 20.0, 30.0};
    const std::vector<double> magnitudes{10.0, 2.0, 3.0, 9.0};

    const auto peaks = find_peaks(
        frequencies, magnitudes, 0.2, PeakDetectionMode::LocalMaxima);

    assert(peaks.empty());
}

void test_detect_dominant_peaks_zero_max_count() {
    const Spectrum spectrum{
        .frequencies = {0.0, 10.0, 20.0},
        .magnitudes = {1.0, 5.0, 3.0}
    };

    const auto peaks = detect_dominant_peaks(
        spectrum, 0.5, PeakDetectionMode::ThresholdOnly, 0);

    assert(peaks.empty());
}

void test_detect_dominant_peaks_empty_spectrum() {
    const Spectrum spectrum{};

    const auto peaks = detect_dominant_peaks(
        spectrum, 0.5, PeakDetectionMode::ThresholdOnly, 3);

    assert(peaks.empty());
}

void test_detect_dominant_peaks_sorted_by_magnitude() {
    const Spectrum spectrum{
        .frequencies = {0.0, 10.0, 20.0, 30.0, 40.0, 50.0},
        .magnitudes = {1.0, 9.0, 2.0, 7.0, 3.0, 8.0}
    };

    const auto peaks = detect_dominant_peaks(
        spectrum, 0.7, PeakDetectionMode::ThresholdOnly, 3);

    assert(peaks.size() == 3);

    assert(peaks[0].frequency == 10.0);
    assert(peaks[0].magnitude == 9.0);

    assert(peaks[1].frequency == 50.0);
    assert(peaks[1].magnitude == 8.0);

    assert(peaks[2].frequency == 30.0);
    assert(peaks[2].magnitude == 7.0);
}

void test_detect_dominant_peaks_respects_max_count() {
    const Spectrum spectrum{
        .frequencies = {0.0, 10.0, 20.0, 30.0, 40.0},
        .magnitudes = {1.0, 9.0, 8.0, 7.0, 6.0}
    };

    const auto peaks = detect_dominant_peaks(
        spectrum, 0.5, PeakDetectionMode::ThresholdOnly, 2);

    assert(peaks.size() == 2);
    assert(peaks[0].frequency == 10.0);
    assert(peaks[1].frequency == 20.0);
}

void test_detect_dominant_peaks_local_maxima_mode() {
    const Spectrum spectrum{
        .frequencies = {0.0, 10.0, 20.0, 30.0, 40.0, 50.0, 60.0},
        .magnitudes = {1.0, 6.0, 10.0, 5.0, 9.0, 4.0, 1.0}
    };

    const auto peaks = detect_dominant_peaks(
        spectrum, 0.5, PeakDetectionMode::LocalMaxima, 5);

    assert(peaks.size() == 2);

    assert(peaks[0].frequency == 20.0);
    assert(peaks[0].magnitude == 10.0);

    assert(peaks[1].frequency == 40.0);
    assert(peaks[1].magnitude == 9.0);
}

} // namespace

int main() {
    test_find_peaks_empty_input();
    test_find_peaks_size_mismatch();
    test_find_peaks_invalid_negative_threshold();
    test_find_peaks_invalid_threshold_above_one();
    test_find_peaks_threshold_only_mode();
    test_find_peaks_local_maxima_mode();
    test_find_peaks_local_maxima_too_few_samples();
    test_find_peaks_local_maxima_ignores_edges();
    test_detect_dominant_peaks_zero_max_count();
    test_detect_dominant_peaks_empty_spectrum();
    test_detect_dominant_peaks_sorted_by_magnitude();
    test_detect_dominant_peaks_respects_max_count();
    test_detect_dominant_peaks_local_maxima_mode();

    return 0;
}
