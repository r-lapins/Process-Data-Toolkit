#include "pdt/pipeline/wav_analysis_service.h"

#include <gtest/gtest.h>
#include <cmath>
#include <numbers>
#include <vector>

namespace {

std::vector<double> make_sine(double frequency, double sample_rate, std::size_t count)
{
    std::vector<double> signal;
    signal.reserve(count);

    for (std::size_t n = 0; n < count; ++n) {
        const double t = static_cast<double>(n) / sample_rate;
        signal.push_back(std::sin(2.0 * std::numbers::pi_v<double> * frequency * t));
    }

    return signal;
}

} // namespace

TEST(WavAnalysisServiceTest, ReturnsSpectrumAndPeaks)
{
    constexpr double sample_rate = 1024.0;
    constexpr std::size_t size = 1024;

    const auto signal = make_sine(128.0, sample_rate, size);

    pdt::WavAnalysisRequest request{
        .samples = signal,
        .sample_rate = sample_rate,
        .settings = {
            .algorithm = pdt::SpectrumAlgorithm::Fft,
            .peak_mode = pdt::PeakDetectionMode::LocalMaxima,
            .window = pdt::WindowType::None,
            .max_peaks = 5,
            .from = 0,
            .window_size = size,
            .threshold = 0.5
        }
    };

    const auto result = pdt::analyze_wav(request);

    ASSERT_FALSE(result.analysis.spectrum.frequencies.empty());
    ASSERT_FALSE(result.analysis.spectrum.magnitudes.empty());
    ASSERT_FALSE(result.analysis.top_peaks.empty());
    EXPECT_NEAR(result.analysis.top_peaks.front().frequency, 128.0, 1e-9);
}

TEST(WavAnalysisServiceTest, ThrowsOnInvalidSampleRate)
{
    const std::vector<double> signal(1024, 0.0);

    pdt::WavAnalysisRequest request{
        .samples = signal,
        .sample_rate = 0.0
    };

    EXPECT_THROW((void)pdt::analyze_wav(request), std::invalid_argument);
}

TEST(WavAnalysisServiceTest, ThrowsOnOutOfRangeFrom)
{
    const std::vector<double> signal(1024, 0.0);

    pdt::WavAnalysisRequest request{
        .samples = signal,
        .sample_rate = 1024.0,
        .settings = {
            .from = 2048
        }
    };

    EXPECT_THROW((void)pdt::analyze_wav(request), std::out_of_range);
}
