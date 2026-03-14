#pragma once

#include "pdt/peak_detection.h"
#include "pdt/window.h"

#include <cstddef>
#include <iosfwd>
#include <string>

namespace spectrum_app {

enum class SpectrumAlgorithm {
    Auto,
    Dft,
    Fft
};

struct CliOptions {
    std::string input_path;
    pdt::WindowType window{pdt::WindowType::Hann};
    bool use_window{true};
    std::size_t from{0};
    std::size_t bins{1024};
    double threshold{0.4};
    pdt::PeakDetectionMode peak_mode{pdt::PeakDetectionMode::LocalMaxima};
    std::size_t top{10};
    SpectrumAlgorithm algorithm{SpectrumAlgorithm::Auto};
    bool help_requested{false};
};

void print_help(std::ostream& os);
bool parse_cli(int argc, const char* const* argv, CliOptions& options, std::ostream& err);

const char* to_string(pdt::WindowType type);
const char* to_string(pdt::PeakDetectionMode mode);
const char* to_string(SpectrumAlgorithm algorithm);

} // namespace spectrum_app
