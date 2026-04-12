#pragma once

#include "pdt/dsp/peak_detection.h"
#include "pdt/dsp/window.h"

#include <cstddef>
#include <ostream>
#include <string>

namespace wav_app {

struct CliOptions {
    std::string input_path;
    std::size_t from{0};
    std::size_t windowSize{1024};
    pdt::WindowType window{pdt::WindowType::Hann};
    pdt::SpectrumAlgorithm algorithm{pdt::SpectrumAlgorithm::Auto};
    pdt::PeakDetectionMode peak_mode{pdt::PeakDetectionMode::LocalMaxima};
    double threshold{0.4};
    std::size_t top{10};
    bool use_window{true};
    bool help_requested{false};
    std::string output_csv_path;
    std::string output_report_path;
};

void print_help(std::ostream& os);
bool parse_cli(int argc, const char* const* argv, CliOptions& options, std::ostream& err);

const char* to_string(pdt::WindowType type);
const char* to_string(pdt::PeakDetectionMode mode);
const char* to_string(pdt::SpectrumAlgorithm algorithm);

} // namespace wav_app
