#pragma once

#include <vector>

namespace pdt {

struct Spectrum {
    std::vector<double> frequencies;
    std::vector<double> magnitudes;
};

} // namespace pdt
