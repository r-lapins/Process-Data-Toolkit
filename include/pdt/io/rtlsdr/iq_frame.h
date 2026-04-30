#pragma once

#include <complex>
#include <cstdint>
#include <vector>

namespace pdt {

struct IqFrame {
    std::vector<std::complex<float>> samples;
    std::uint32_t sample_rate{};
    std::uint64_t sequence{};
};

} // namespace pdt
