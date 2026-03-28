#include "pdt/wav/peak_detection.h"

#include <cassert>
#include <vector>

namespace {

using namespace pdt;

using enum PeakDetectionMode;

void test_find_peaks_empty_input() {
    const Spectrum spectrum{
        .frequencies{},
        .magnitudes{}
    };

    const auto peaks = find_peaks(
        spectrum, 0.8, ThresholdOnly);

    assert(peaks.empty());
}

void test_find_peaks_size_mismatch() {
    const Spectrum spectrum{
        .frequencies{0.0, 10.0, 20.0},
        .magnitudes{1.0, 2.0}
    };

    const auto peaks = find_peaks(
        spectrum, 0.8, ThresholdOnly);

    assert(peaks.empty());
}

void test_find_peaks_invalid_negative_threshold() {
    const Spectrum spectrum{
        .frequencies{0.0, 10.0, 20.0},
        .magnitudes{1.0, 2.0, 3.0}
    };

    const auto peaks = find_peaks(
        spectrum, -0.1, ThresholdOnly);

    assert(peaks.empty());
}

void test_find_peaks_invalid_threshold_above_one() {
    const Spectrum spectrum{
        .frequencies{0.0, 10.0, 20.0},
        .magnitudes{1.0, 2.0, 3.0}
    };

    const auto peaks = find_peaks(
        spectrum, 1.1, ThresholdOnly);

    assert(peaks.empty());
}

void test_find_peaks_threshold_only_mode() {
    const Spectrum spectrum{
        .frequencies{0.0, 10.0, 20.0, 30.0, 40.0},
        .magnitudes{1.0, 8.5, 10.0, 8.7, 2.0}
    };

    const auto peaks = find_peaks(
        spectrum, 0.8, ThresholdOnly);

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
    const Spectrum spectrum{
        .frequencies{0.0, 10.0, 20.0, 30.0, 40.0},
        .magnitudes{1.0, 8.5, 10.0, 8.7, 2.0}
    };

    const auto peaks = find_peaks(
        spectrum, 0.8, LocalMaxima);

    assert(peaks.size() == 1);
    assert(peaks[0].index == 2);
    assert(peaks[0].frequency == 20.0);
    assert(peaks[0].magnitude == 10.0);
}

void test_find_peaks_local_maxima_too_few_samples() {
    const Spectrum spectrum{
        .frequencies{0.0, 10.0},
        .magnitudes{1.0, 2.0}
    };

    const auto peaks = find_peaks(
        spectrum, 0.5, LocalMaxima);

    assert(peaks.empty());
}

void test_find_peaks_local_maxima_ignores_edges() {
    const Spectrum spectrum{
        .frequencies{0.0, 10.0, 20.0, 30.0},
        .magnitudes{10.0, 2.0, 3.0, 9.0}
    };

    const auto peaks = find_peaks(
        spectrum, 0.2, LocalMaxima);

    assert(peaks.empty());
}

void test_detect_dominant_peaks_zero_max_count() {
    const Spectrum spectrum{
        .frequencies = {0.0, 10.0, 20.0},
        .magnitudes = {1.0, 5.0, 3.0}
    };

    const auto peaks = detect_dominant_peaks(
        spectrum, 0.5, ThresholdOnly, 0);

    assert(peaks.empty());
}

void test_detect_dominant_peaks_empty_spectrum() {
    const Spectrum spectrum{};

    const auto peaks = detect_dominant_peaks(
        spectrum, 0.5, ThresholdOnly, 3);

    assert(peaks.empty());
}

void test_detect_dominant_peaks_sorted_by_magnitude() {
    const Spectrum spectrum{
        .frequencies = {0.0, 10.0, 20.0, 30.0, 40.0, 50.0},
        .magnitudes = {1.0, 9.0, 2.0, 7.0, 3.0, 8.0}
    };

    const auto peaks = detect_dominant_peaks(
        spectrum, 0.7, ThresholdOnly, 3);

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
        spectrum, 0.5, ThresholdOnly, 2);

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
        spectrum, 0.5, LocalMaxima, 5);

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
