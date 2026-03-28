#pragma once

#include "pdt/csv/anomaly.h"

#include <chrono>
#include <optional>
#include <ostream>
#include <string>
#include <cstddef>

namespace csv_app {

struct CliOptions {
    std::string input_path;
    std::optional<std::string> sensor;
    std::optional<std::chrono::sys_seconds> from;
    std::optional<std::chrono::sys_seconds> to;
    std::optional<std::string> output_path;
    std::optional<std::string> output_marked_csv_path;

    std::optional<double> anomaly_threshold;
    pdt::AnomalyMethod anomaly_method{pdt::AnomalyMethod::ZScore};

    std::size_t top{10};
    bool per_sensor{false};
    bool show_skipped{false};
    bool help{false};
};

void print_help(std::ostream& os);
bool parse_args(int argc, const char* const* argv, CliOptions& out, std::ostream& err);


} // namespace csv_app
