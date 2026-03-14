#include "spectrum_cli_args.h"

#include <cstdlib>
#include <optional>
#include <ostream>
#include <string>
#include <string_view>

namespace spectrum_app {
namespace {

std::optional<std::string_view> get_value(int& i, int argc, const char* const* argv) {
    if (i + 1 >= argc) { return std::nullopt; }

    ++i;
    return std::string_view{argv[i]};
}

std::optional<pdt::WindowType> parse_window_type(std::string_view value) {
    if (value == "hann") { return pdt::WindowType::Hann; }
    if (value == "hamming") { return pdt::WindowType::Hamming; }
    return std::nullopt;
}

std::optional<pdt::PeakDetectionMode> parse_peak_mode(std::string_view value) {
    if (value == "threshold-only") { return pdt::PeakDetectionMode::ThresholdOnly; }
    if (value == "local-maxima") { return pdt::PeakDetectionMode::LocalMaxima; }
    return std::nullopt;
}

std::optional<SpectrumAlgorithm> parse_algorithm(std::string_view value) {
    if (value == "auto") { return SpectrumAlgorithm::Auto; }
    if (value == "dft") { return SpectrumAlgorithm::Dft; }
    if (value == "fft") { return SpectrumAlgorithm::Fft; }
    return std::nullopt;
}

bool parse_size_t(std::string_view text, std::size_t& out) {
    if (!text.empty() && text.front() == '-') { return false; }

    try {
        std::size_t pos = 0;
        const auto value = std::stoull(std::string{text}, &pos);
        if (pos != text.size()) { return false; }
        out = static_cast<std::size_t>(value);
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_double(std::string_view text, double& out) {
    if (!text.empty() && text.front() == '-') { return false; }

    try {
        std::size_t pos = 0;
        out = std::stod(std::string{text}, &pos);
        return pos == text.size();
    } catch (...) {
        return false;
    }
}

} // namespace

void print_help(std::ostream& os) {
    os
        << "Usage:\n"
        << "  spectrum_demo [options] <input.wav>\n\n"
        << "Options:\n"
        << "  --window <none|hann|hamming>         Window function to apply (default: hann)\n"
        << "  --from <index>                       Start sample index (default: 0)\n"
        << "  --bins <count>                       Number of samples to analyze (default: 1024)\n"
        << "  --threshold <0..1>                   Peak detection threshold ratio (default: 0.4)\n"
        << "  --mode <threshold-only|local-maxima> Peak detection mode (default: local-maxima)\n"
        << "  --top <count>                        Number of dominant peaks to print (default: 10)\n"
        << "  --algorithm <auto|dft|fft>           Spectrum algorithm (default: auto)\n"
        << "  --help                               Show this help message\n";
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
bool parse_cli(int argc, const char* const* argv, CliOptions& options, std::ostream& err) {
    for (int i = 1; i < argc; ++i) {
        const std::string_view arg{argv[i]};

        if (arg == "--help" || arg == "-h") {
            options.help_requested = true;
            return true;
        }

        if (arg == "--window") {
            const auto value = get_value(i, argc, argv);
            if (!value) {
                err << "Missing value for --window\n";
                return false;
            }

            if (*value == "none") {
                options.use_window = false;
                continue;
            }

            const auto parsed = parse_window_type(*value);
            if (!parsed.has_value()) {
                err << "Invalid value for --window: " << *value << '\n';
                return false;
            }

            options.window = *parsed;
            options.use_window = true;
            continue;
        }

        if (arg == "--from") {
            const auto value = get_value(i, argc, argv);
            if (!value) {
                err << "Missing value for --from\n";
                return false;
            }

            if (!parse_size_t(*value, options.from)) {
                err << "Invalid value for --from: " << *value << '\n';
                return false;
            }
            continue;
        }

        if (arg == "--bins") {
            const auto value = get_value(i, argc, argv);
            if (!value) {
                err << "Missing value for --bins\n";
                return false;
            }

            if (!parse_size_t(*value, options.bins) || options.bins == 0) {
                err << "Invalid value for --bins: " << *value << '\n';
                return false;
            }
            continue;
        }

        if (arg == "--threshold") {
            const auto value = get_value(i, argc, argv);
            if (!value) {
                err << "Missing value for --threshold\n";
                return false;
            }

            if (!parse_double(*value, options.threshold) ||
                options.threshold < 0.0 || options.threshold > 1.0) {
                err << "Invalid value for --threshold: " << *value << '\n';
                return false;
            }
            continue;
        }

        if (arg == "--mode") {
            const auto value = get_value(i, argc, argv);
            if (!value) {
                err << "Missing value for --mode\n";
                return false;
            }

            const auto parsed = parse_peak_mode(*value);
            if (!parsed.has_value()) {
                err << "Invalid value for --mode: " << *value << '\n';
                return false;
            }

            options.peak_mode = *parsed;
            continue;
        }

        if (arg == "--top") {
            const auto value = get_value(i, argc, argv);
            if (!value) {
                err << "Missing value for --top\n";
                return false;
            }

            if (!parse_size_t(*value, options.top) || options.top == 0) {
                err << "Invalid value for --top: " << *value << '\n';
                return false;
            }
            continue;
        }

        if (arg == "--algorithm") {
            const auto value = get_value(i, argc, argv);
            if (!value) {
                err << "Missing value for --algorithm\n";
                return false;
            }

            const auto parsed = parse_algorithm(*value);
            if (!parsed.has_value()) {
                err << "Invalid value for --algorithm: " << *value << '\n';
                return false;
            }

            options.algorithm = *parsed;
            continue;
        }

        if (!arg.empty() && arg.front() == '-') {
            err << "Unknown option: " << arg << '\n';
            err << "Use --help to see available options.\n";
            return false;
        }

        if (!options.input_path.empty()) {
            err << "Only one input WAV file may be provided.\n";
            return false;
        }

        options.input_path = std::string{arg};
    }

    if (options.input_path.empty() && !options.help_requested) {
        err << "Missing input WAV file.\n";
        return false;
    }

    return true;
}

const char* to_string(pdt::WindowType type) {
    switch (type) {
    case pdt::WindowType::Hann:
        return "hann";
    case pdt::WindowType::Hamming:
        return "hamming";
    }
    return "unknown";
}

const char* to_string(pdt::PeakDetectionMode mode) {
    switch (mode) {
    case pdt::PeakDetectionMode::ThresholdOnly:
        return "threshold-only";
    case pdt::PeakDetectionMode::LocalMaxima:
        return "local-maxima";
    }
    return "unknown";
}

const char* to_string(SpectrumAlgorithm algorithm) {
    switch (algorithm) {
    case SpectrumAlgorithm::Auto:
        return "auto";
    case SpectrumAlgorithm::Dft:
        return "dft";
    case SpectrumAlgorithm::Fft:
        return "fft";
    }
    return "unknown";
}

} // namespace spectrum_app
