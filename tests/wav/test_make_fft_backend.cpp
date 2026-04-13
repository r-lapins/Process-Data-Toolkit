#include "pdt/compute/make_fft_backend.h"

#include <gtest/gtest.h>

TEST(BackendFactoryTest, CpuAlgorithmsCreateCpuBackend)
{
    auto auto_backend = pdt::create_fft_backend(pdt::SpectrumAlgorithm::Auto);
    auto dft_backend  = pdt::create_fft_backend(pdt::SpectrumAlgorithm::Dft);
    auto fft_backend  = pdt::create_fft_backend(pdt::SpectrumAlgorithm::Fft);

    ASSERT_NE(auto_backend, nullptr);
    ASSERT_NE(dft_backend, nullptr);
    ASSERT_NE(fft_backend, nullptr);

    EXPECT_FALSE(auto_backend->is_gpu());
    EXPECT_FALSE(dft_backend->is_gpu());
    EXPECT_FALSE(fft_backend->is_gpu());
}

TEST(BackendFactoryTest, AvailabilityMatchesBuild)
{
    EXPECT_TRUE(pdt::is_algorithm_available(pdt::SpectrumAlgorithm::Auto));
    EXPECT_TRUE(pdt::is_algorithm_available(pdt::SpectrumAlgorithm::Dft));
    EXPECT_TRUE(pdt::is_algorithm_available(pdt::SpectrumAlgorithm::Fft));

#ifdef PDT_ENABLE_CUDA
    EXPECT_TRUE(pdt::is_algorithm_available(pdt::SpectrumAlgorithm::cuFft));
#else
    EXPECT_FALSE(pdt::is_algorithm_available(pdt::SpectrumAlgorithm::cuFft));
#endif
}
