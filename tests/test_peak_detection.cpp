#include "pdt/peak_detection.h"

#include <cassert>
#include <vector>

int main() {
    using namespace pdt;

        // 1. Empty input
    {
        const std::vector<double> frequencies{};
        const std::vector<double> magnitudes{};

        const auto peaks = find_peaks(frequencies, magnitudes, 0.8, PeakDetectionMode::ThresholdOnly);
        assert(peaks.empty());
    }

        // 2. Size mismatch
    {
        const std::vector<double> frequencies{0.0, 1.0, 2.0};
        const std::vector<double> magnitudes{1.0, 2.0};

        const auto peaks = find_peaks(frequencies, magnitudes, 0.8, PeakDetectionMode::ThresholdOnly);
        assert(peaks.empty());
    }

        // 3. Simple threshold-based detection
    {
        const std::vector<double> frequencies{0.0, 10.0, 20.0, 30.0, 40.0};
        const std::vector<double> magnitudes{1.0, 3.0, 10.0, 7.0, 2.0};

        const auto peaks = find_peaks(frequencies, magnitudes, 0.8, PeakDetectionMode::ThresholdOnly);

        // max = 10, threshold = 8 => only 20 Hz should remain
        assert(peaks.size() == 1);
        assert(peaks[0].index == 2);
        assert(peaks[0].frequency == 20.0);
        assert(peaks[0].magnitude == 10.0);
    }

           // 4. LocalMaxima mode
    {
        const std::vector<double> frequencies{0.0, 10.0, 20.0, 30.0, 40.0};
        const std::vector<double> magnitudes{1.0, 8.5, 10.0, 8.7, 2.0};

        const auto peaks = find_peaks(frequencies, magnitudes, 0.8, PeakDetectionMode::LocalMaxima);

        assert(peaks.size() == 1);
        assert(peaks[0].index == 2);
        assert(peaks[0].frequency == 20.0);
        assert(peaks[0].magnitude == 10.0);
    }

    return 0;
}
