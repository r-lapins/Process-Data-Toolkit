#include "spectrum_cli_args.h"
#include "cli_common.h"

#include <optional>
#include <ostream>
#include <string>
#include <string_view>

namespace spectrum_app {
namespace {

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
    cli_common::ArgReader args(argc, argv);

    while (args.has_next()) {
        const std::string_view arg = args.next();

        if (arg == "--help" || arg == "-h") {
            options.help_requested = true;
            return true;
        }

        if (arg == "--window") {
            const auto value = args.value();
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
            const auto value = args.value();
            if (!value) {
                err << "Missing value for --from\n";
                return false;
            }

            if (!cli_common::parse_size_t(*value, options.from)) {
                err << "Invalid value for --from: " << *value << '\n';
                return false;
            }
            continue;
        }

        if (arg == "--bins") {
            const auto value = args.value();
            if (!value) {
                err << "Missing value for --bins\n";
                return false;
            }

            if (!cli_common::parse_size_t(*value, options.bins) || options.bins == 0) {
                err << "Invalid value for --bins: " << *value << '\n';
                return false;
            }
            continue;
        }

        if (arg == "--threshold") {
            const auto value = args.value();
            if (!value) {
                err << "Missing value for --threshold\n";
                return false;
            }

            if (!cli_common::parse_double(*value, options.threshold) ||
                options.threshold < 0.0 || options.threshold > 1.0) {
                err << "Invalid value for --threshold: " << *value << '\n';
                return false;
            }
            continue;
        }

        if (arg == "--mode") {
            const auto value = args.value();
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
            const auto value = args.value();
            if (!value) {
                err << "Missing value for --top\n";
                return false;
            }

            if (!cli_common::parse_size_t(*value, options.top) || options.top == 0) {
                err << "Invalid value for --top: " << *value << '\n';
                return false;
            }
            continue;
        }

        if (arg == "--algorithm") {
            const auto value = args.value();
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

        if (cli_common::is_option(arg)) {
            return cli_common::fail_unknown_option(arg, err);
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
