#pragma once
#include "types.h"
#include <chrono>
#include <optional>
#include <string>
// #include <string_view>
#include <vector>

namespace pdt {

struct FilterOptions
{
    std::optional<std::string> sensor; // exact math for now
    std::optional<std::chrono::sys_seconds> from;   // inclusive
    std::optional<std::chrono::sys_seconds> to;     // inclusive
};

std::vector<Sample> filter_samples(const std::vector<Sample>& samples, const FilterOptions& opt);

} // namespace pdt
