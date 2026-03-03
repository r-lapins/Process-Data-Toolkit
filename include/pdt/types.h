#pragma once
#include <chrono>
#include <string>

namespace pdt {

struct Sample {
    std::chrono::sys_seconds timestamp;
    std::string sensor;
    double value{};
};

struct Stats {
    double mean{};
    double min{};
    double max{};
    double stddev{};
    std::size_t count{};
};

struct FilterOptions
{
    std::optional<std::string> sensor;
    std::optional<std::chrono::sys_seconds> from;   // inclusive
    std::optional<std::chrono::sys_seconds> to;     // inclusive
};

} // namespace pdt
