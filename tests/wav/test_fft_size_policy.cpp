#include "pdt/compute/fft_size_policy.h"
#include <gtest/gtest.h>

TEST(FftSizePolicyTest, CpuOnlyRecommended)
{
    const auto v = pdt::get_fft_size_options(
        pdt::SpectrumAlgorithm::Fft,
        8192,
        true);

    ASSERT_FALSE(v.empty());

    for (const auto& o : v) {
        EXPECT_TRUE(o.recommended);
        EXPECT_FALSE(o.advanced);
    }
}

TEST(FftSizePolicyTest, CudaAdvancedToggle)
{
    const auto no_adv = pdt::get_fft_size_options(
        pdt::SpectrumAlgorithm::cuFft,
        8192,
        false);

    const auto adv = pdt::get_fft_size_options(
        pdt::SpectrumAlgorithm::cuFft,
        8192,
        true);

    ASSERT_FALSE(no_adv.empty());
    ASSERT_FALSE(adv.empty());

    for (const auto& o : no_adv) {
        EXPECT_FALSE(o.advanced);
    }

    bool has_adv = false;
    for (const auto& o : adv) {
        if (o.advanced) {
            has_adv = true;
        }
    }

    EXPECT_TRUE(has_adv);
}
