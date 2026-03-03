#include "cli_args.h"
#include "pdt/time.h"

#include <optional>
#include <string>
#include <string_view>

namespace pdt_app {

namespace {

// Helper: consume "--key value" from
std::optional<std::string_view> get_value(int& i, int argc, const char* const* argv) {
    if (i + 1 >= argc) return std::nullopt;
    ++i;
    return std::string_view{argv[i]};
}

} // namespace

void print_help(std::ostream& os) {
    os <<
        R"(Proces Data Toolkit

Usage:
    pdt_cli --in <file.csv> [--sensor <name>] [--from <ISO>] [--to <ISO>]

Options:
    --in          Path to CSV file (required)
    --sensor      Filter by exact sensor name
    --per-sensor  Output per-sensor statistics (mutually exclusive with --sensor)
    --from        Inclusive time lower bound, ISO: YYYY-MM-DDTHH:MM:SS
    --to          Inclusive time upper bound, ISO: YYYY-MM-DDTHH:MM:SS
    --out         Write JSON report to file
    --help        Show this help
)";
}

bool parse_args(int argc, const char* const* argv, CliOptions& out, std::ostream& err) {
    for (int i = 1; i < argc; ++i) {
        std::string_view a{argv[i]};

        if (a == "--") {
            if (i + 1 < argc) {
                err << "Unexpected positional arguments after --\n";
                return false;
            }
            return true;
        }

        if (a == "--help" || a == "-h") {
            out.help = true;
            return true;
        }

        if (a == "--in") {
            auto v = get_value(i, argc, argv);
            if (!v) { err << "Missing value for --in\n"; return false; }
            out.input_path = std::string{*v};
            continue;
        }

        if (a == "--sensor") {
            auto v = get_value(i, argc, argv);
            if (!v) { err << "Missing value for --sensor\n"; return false; }
            out.sensor = std::string{*v};
            continue;
        }

        if (a == "--per-sensor") {
            out.per_sensor = true;
            continue;
        }

        if (a == "--from") {
            auto v = get_value(i, argc, argv);
            if (!v) { err << "Missing value for --from\n"; return false; }
            auto ts = pdt::parse_iso8601(*v);
            if (!ts) { err << "Invalid --from timestamp (expected YYYY-MM-DDTHH:MM:SS)\n"; return false; }
            out.from = *ts;
            continue;
        }

        if (a == "--to") {
            auto v = get_value(i, argc, argv);
            if (!v) { err << "Missing value for --to\n"; return false; }
            auto ts = pdt::parse_iso8601(*v);
            if (!ts) { err << "Invalid --to timestamp (expected YYYY-MM-DDTHH:MM:SS)\n"; return false; }
            out.to = *ts;
            continue;
        }

        if (a == "--out") {
            auto v = get_value(i, argc, argv);
            if (!v) { err << "Missing value for --out\n"; return false; }
            out.output_path = std::string{*v};
            continue;
        }

        err << "Unknown argument: " << a << "\n";
        err << "Use --help to see available options.\n";
        return false;
    }

    return true;
}

} // namespace pdt_app
