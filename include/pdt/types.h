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

} // namespace pdt
