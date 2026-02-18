#pragma once
#include "types.h"
#include <istream>
#include <vector>

namespace pdt {

struct ImportResult {
    std::vector<Sample> samples;
    std::size_t parsed_ok{};
    std::size_t skipped{};
};

ImportResult read_csv(std::istream& input);

} // namespace pdt
