#pragma once

#include "pdt/dsp/spectrum.h"

#include <cstddef>
#include <span>
#include <vector>

namespace pdt {

struct FftSizeOption {
    std::size_t size{};
    bool recommended{};
    bool advanced{};
};

bool is_cuda_supported_fft_size(std::size_t n);

std::span<const std::size_t> all_power_of_two_sizes();
std::span<const std::size_t> all_gpu_preferred_sizes();

std::vector<std::size_t> best_fft_sizes(std::size_t sample_count);
std::vector<std::size_t> best_cufft_sizes(std::size_t sample_count);

std::vector<std::size_t> make_supported_fft_sizes(std::size_t sample_count,
                                                  bool cuda_enabled,
                                                  bool include_cuda_supported);

std::vector<FftSizeOption> get_fft_size_options(SpectrumAlgorithm algorithm,
                                                std::size_t available_samples,
                                                bool include_advanced);

} // namespace pdt
