#pragma once

#include <vector>

namespace pdt {

enum class SpectrumAlgorithm {
    Auto,
    Dft,
    Fft
};

struct Spectrum {
    std::vector<double> frequencies;
    std::vector<double> magnitudes;
};

} // namespace pdt
