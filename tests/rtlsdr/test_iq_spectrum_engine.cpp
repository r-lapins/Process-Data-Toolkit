#include "pdt/compute/cpu_fft_backend.h"
#include "pdt/pipeline/iq_spectrum_engine.h"

#include <gtest/gtest.h>

#include <cmath>
#include <numbers>
#include <vector>

namespace {

pdt::IqFrame make_frame(double frequency, double sample_rate, std::size_t count)
{
    pdt::IqFrame frame;
    frame.sample_rate = static_cast<std::uint32_t>(sample_rate);
    frame.sequence = 42;
    frame.samples.reserve(count);

    for (std::size_t n = 0; n < count; ++n) {
        const double t = static_cast<double>(n) / sample_rate;
        const double phase = 2.0 * std::numbers::pi_v<double> * frequency * t;

        frame.samples.emplace_back(
            static_cast<float>(std::cos(phase)),
            static_cast<float>(std::sin(phase)));
    }

    return frame;
}

} // namespace

TEST(IqSpectrumEngineTest, DetectsIqPeak)
{
    constexpr double sample_rate = 1024.0;
    constexpr std::size_t size = 1024;

    auto frame = make_frame(128.0, sample_rate, size);

    pdt::CpuFftBackend backend;
    pdt::IqSpectrumEngine engine{backend};

    pdt::SpectrumAnalysisOptions options;
    options.window = pdt::WindowType::None;
    options.threshold = 0.5;
    options.peak_mode = pdt::PeakDetectionMode::LocalMaxima;
    options.max_peaks = 1;

    const auto result = engine.process(frame, options);

    ASSERT_FALSE(result.top_peaks.empty());
    EXPECT_NEAR(result.top_peaks.front().frequency, 128.0, 1.0);
}