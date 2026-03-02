#pragma once
#include <chrono>
#include <optional>
#include <ostream>
#include <string>

namespace pdt_app {

struct CliOptions {
    std::string input_path;
    std::optional<std::string> sensor;
    std::optional<std::chrono::sys_seconds> from;
    std::optional<std::chrono::sys_seconds> to;
    std::optional<std::string> output_path;
    bool help{false};
};

void print_help(std::ostream& os);
bool parse_args(int argc, const char* const* argv, CliOptions& out, std::ostream& err);


} // namespace pdt_app
