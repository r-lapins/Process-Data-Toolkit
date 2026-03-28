#include "csv_cli_args.h"
#include "common_cli.h"
#include "pdt/csv/time.h"

#include <string>
#include <string_view>

namespace csv_app {

namespace {

std::optional<pdt::AnomalyMethod> parse_anomaly_method(std::string_view v) {
    using enum pdt::AnomalyMethod;
    if (v == "zscore") return ZScore;
    if (v == "iqr")    return IQR;
    if (v == "mad")    return MAD;
    return std::nullopt;
}

} // namespace

void print_help(std::ostream& os) {
    os <<
        R"(Proces Data Toolkit

Usage:
    pdt_cli --in <file.csv> [--sensor <name>] [--from <ISO>] [--to <ISO>]

Options:
    --in               Path to CSV file (required)
    --sensor           Filter by exact sensor name
    --per-sensor       Output per-sensor statistics (mutually exclusive with --sensor)
    --from             Inclusive time lower bound, ISO: YYYY-MM-DDTHH:MM:SS
    --to               Inclusive time upper bound, ISO: YYYY-MM-DDTHH:MM:SS
    --out              Write JSON report to file
    --z <val>          Enable anomaly detection and set threshold
    --method           Anomaly method: zscore | iqr | mad (default: zscore)
    --top <N>          Max anomalies to list
    --skipped          Print skipped CSV rows to stderr
    --help             Show this help
)";
}

bool parse_args(int argc, const char* const* argv, CliOptions& out, std::ostream& err) {
    cli_common::ArgReader args(argc, argv);

    while (args.has_next()) {
        const std::string_view a = args.next();

        if (a == "--") {
            if (args.has_remaining()) {
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
            const auto v = args.value();
            if (!v) { err << "Missing value for --in\n"; return false; }
            out.input_path = std::string{*v};
            continue;
        }

        if (a == "--sensor") {
            const auto v = args.value();
            if (!v) { err << "Missing value for --sensor\n"; return false; }
            out.sensor = std::string{*v};
            continue;
        }

        if (a == "--per-sensor") {
            out.per_sensor = true;
            continue;
        }

        if (a == "--from") {
            const auto v = args.value();
            if (!v) { err << "Missing value for --from\n"; return false; }

            const auto ts = pdt::parse_iso8601(*v);
            if (!ts) {
                err << "Invalid --from timestamp (expected YYYY-MM-DDTHH:MM:SS)\n";
                return false;
            }

            out.from = *ts;
            continue;
        }

        if (a == "--to") {
            const auto v = args.value();
            if (!v) { err << "Missing value for --to\n"; return false; }

            const auto ts = pdt::parse_iso8601(*v);
            if (!ts) {
                err << "Invalid --to timestamp (expected YYYY-MM-DDTHH:MM:SS)\n";
                return false;
            }

            out.to = *ts;
            continue;
        }

        if (a == "--out") {
            const auto v = args.value();
            if (!v) { err << "Missing value for --out\n"; return false; }
            out.output_path = std::string{*v};
            continue;
        }

        if (a == "--z") {
            const auto v = args.value();
            if (!v) { err << "Missing value for --z\n"; return false; }

            double parsed{};
            if (!cli_common::parse_double(*v, parsed) || parsed <= 0.0) {
                err << "Invalid value for --z\n";
                return false;
            }

            out.anomaly_threshold = parsed;
            continue;
        }

        if (a == "--method") {
            const auto v = args.value();
            if (!v) { err << "Missing value for --method\n"; return false; }

            const auto method = parse_anomaly_method(*v);
            if (!method) {
                err << "Invalid value for --method (expected: zscore, iqr, mad)\n";
                return false;
            }

            out.anomaly_method = *method;
            continue;
        }

        if (a == "--top") {
            const auto v = args.value();
            if (!v) { err << "Missing value for --top\n"; return false; }

            std::size_t n = 0;
            if (!cli_common::parse_size_t(*v, n) || n > 25) {
                err << "--top must be in range [0, 25]\n";
                return false;
            }

            out.top = n;
            continue;
        }

        if (a == "--skipped") {
            out.show_skipped = true;
            continue;
        }

        return cli_common::fail_unknown_option(a, err);
    }

    return true;
}

} // namespace pdt_app
