#include "pdt/compute/cpu_fft_backend.h"
#include "pdt/dsp/peak_detection.h"

#include <gtest/gtest.h>

#include <complex>
#include <numbers>
#include <vector>
#include <cmath>

namespace {

// NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
std::vector<std::complex<float>> make_iq_sine(double frequency,
                                              double sample_rate,
                                              std::size_t count)
{
    std::vector<std::complex<float>> signal;
    signal.reserve(count);

    for (std::size_t n = 0; n < count; ++n) {
        const double t = static_cast<double>(n) / sample_rate;
        const double phase = 2.0 * std::numbers::pi_v<double> * frequency * t;

        signal.emplace_back(static_cast<float>(std::cos(phase)),
                            static_cast<float>(std::sin(phase)));
    }

    return signal;
}

} // namespace

TEST(IqFftBackendTest, CpuComputesIqSpectrum)
{
    constexpr double sample_rate = 1024.0;
    constexpr std::size_t size = 1024;

    const auto iq = make_iq_sine(128.0, sample_rate, size);

    pdt::CpuFftBackend backend;
    const auto result = backend.compute_iq_spectrum(iq,
                                                    sample_rate,
                                                    pdt::WindowType::None);

    ASSERT_FALSE(result.spectrum.frequencies.empty());
    ASSERT_FALSE(result.spectrum.magnitudes.empty());
    EXPECT_EQ(result.algorithm, pdt::SpectrumAlgorithm::Fft);

    const auto peaks = pdt::detect_dominant_peaks(result.spectrum,
                                                  0.5,
                                                  pdt::PeakDetectionMode::LocalMaxima,
                                                  1);

    ASSERT_FALSE(peaks.empty());
    EXPECT_NEAR(peaks.front().frequency, 128.0, 1.0);
}

TEST(IqFftBackendTest, CpuDetectsNegativeIqFrequency)
{
    constexpr double sample_rate = 1024.0;
    constexpr std::size_t size = 1024;

    const auto iq = make_iq_sine(-128.0, sample_rate, size);

    pdt::CpuFftBackend backend;
    const auto result = backend.compute_iq_spectrum(
        iq,
        sample_rate,
        pdt::WindowType::None);

    const auto peaks = pdt::detect_dominant_peaks(
        result.spectrum,
        0.5,
        pdt::PeakDetectionMode::LocalMaxima,
        1);

    ASSERT_FALSE(peaks.empty());
    EXPECT_NEAR(peaks.front().frequency, -128.0, 1.0);
}
