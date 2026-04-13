#include "pdt/compute/fft_size_policy.h"

#include <algorithm>

namespace pdt {
namespace {

constexpr std::size_t MIN_FFT_SIZE = 1024;
constexpr std::size_t MAX_FFT_SIZE = 1'048'576;

void filter_min_size(std::vector<std::size_t>& v, std::size_t sample_count)
{
    std::size_t min_size = 2;

    if (sample_count >= 8192) { min_size = MIN_FFT_SIZE; }

    std::erase_if(v, [min_size](std::size_t n) {
        return n < min_size;
    });
}

bool is_power_of_two(std::size_t n) {
    return n > 0 && (n & (n - 1)) == 0;
}

bool is_cuda_supported_impl(std::size_t n) {
    if (n == 0) { return false; }

    // for (const std::size_t f : {std::size_t{2}, std::size_t{3}, std::size_t{5}, std::size_t{7}}) {
    for (const std::size_t f : {std::size_t{2}, std::size_t{7}}) {
        while (n % f == 0) { n /= f; }
    }

    return n == 1;
}

std::vector<std::size_t> build_power_of_two() {
    std::vector<std::size_t> v;
    for (std::size_t n = 1; n <= MAX_FFT_SIZE; n <<= 1) {
        v.push_back(n);
    }
    return v;
}

std::vector<std::size_t> build_cuda_supported() {
    std::vector<std::size_t> v;
    v.reserve(2048);

    for (std::size_t n = 1; n <= MAX_FFT_SIZE; ++n) {
        if (is_cuda_supported_impl(n) && !is_power_of_two(n)) {
            v.push_back(n);
        }
    }

    std::ranges::sort(v);
    return v;
}

} // namespace

bool is_cuda_supported_fft_size(std::size_t n) {
    return is_cuda_supported_impl(n);
}

std::span<const std::size_t> all_power_of_two_sizes() {
    static const std::vector<std::size_t> v = build_power_of_two();
    return v;
}

std::span<const std::size_t> all_gpu_preferred_sizes() {
    static const std::vector<std::size_t> v = build_cuda_supported();
    return v;
}

std::vector<std::size_t> make_supported_fft_sizes(std::size_t sample_count,
                                                  bool cuda_enabled,
                                                  bool include_cuda_supported) {
    std::vector<std::size_t> result;

    const auto p2 = all_power_of_two_sizes();
    auto it = std::ranges::upper_bound(p2, sample_count);
    result.insert(result.end(), p2.begin(), it);

    if (cuda_enabled && include_cuda_supported) {
        const auto cuda = all_gpu_preferred_sizes();
        auto it2 = std::ranges::upper_bound(cuda, sample_count);
        result.insert(result.end(), cuda.begin(), it2);
    }

    std::ranges::sort(result);
    result.erase(std::ranges::unique(result).begin(), result.end());

    return result;
}

std::vector<std::size_t> best_fft_sizes(std::size_t sample_count)
{
    std::vector<std::size_t> result;

    const auto p2 = pdt::all_power_of_two_sizes();
    auto it = std::ranges::upper_bound(p2, sample_count);

    result.insert(result.end(), p2.begin(), it);

    filter_min_size(result, sample_count);

    return result;
}

std::vector<std::size_t> best_cufft_sizes(std::size_t sample_count)
{
    std::vector<std::size_t> result;

    const auto p2 = pdt::all_power_of_two_sizes();
    auto it = std::ranges::upper_bound(p2, sample_count);
    result.insert(result.end(), p2.begin(), it);

    const auto cuda = pdt::all_gpu_preferred_sizes();
    auto it2 = std::ranges::upper_bound(cuda, sample_count);
    result.insert(result.end(), cuda.begin(), it2);

    std::ranges::sort(result);
    result.erase(std::ranges::unique(result).begin(), result.end());

    filter_min_size(result, sample_count);

    return result;
}

} // namespace pdt
