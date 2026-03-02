#include "pdt/dataset.h"
#include "pdt/csv_reader.h"
#include "pdt/time.h"

#include <iostream>
#include <chrono>
#include <fstream>
#include <optional>
#include <string>
#include <string_view>

namespace {

struct CliOptions {
    std::string input_path;
    std::optional<std::string> sensor;
    std::optional<std::chrono::sys_seconds> from;
    std::optional<std::chrono::sys_seconds> to;
    bool help{false};
};

void print_help(std::ostream& os) {
    os <<
R"(Proces Data Toolkit

Usage:
    pdt_cli --in <file.csv> [--sensor <name>] [--from <ISO>] [--to <ISO>]

Options:
    --in        Path to CSV file (required)
    --sensor    Filter by exact sensor name
    --from      Inclusive time lower bound, ISO: YYYY-MM-DDTHH:MM:SS
    --to        Inclusive time upper bound, ISO: YYYY-MM-DDTHH:MM:SS
    --help      Show this help
)";
}

// Helper: consume ""--key value" from
std::optional<std::string_view> get_value(int& i, int argc, char** argv) {
    if (i + 1 >= argc) return std::nullopt;
    ++i;
    return std::string_view{argv[i]};
}

bool parse_args(int argc, char** argv, CliOptions& out, std::ostream& err) {
    for (int i = 1; i < argc; ++i) {
        std::string_view a{argv[i]};

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

        err << "Unknown argument: " << a << "\n";
        return false;
    }

    return true;
}

} // namespace

int main(int argc, char** argv) {
    CliOptions opt{};
    if (!parse_args(argc, argv, opt, std::cerr)) {
        std::cerr << "\n";
        print_help(std::cerr);
        return 2;
    }

    if (opt.help) {
        print_help(std::cout);
        return 0;
    }

    if (opt.input_path.empty()) {
        std::cerr << "Missing required --in <file.csv>\n\n";
        print_help(std::cerr);
        return 2;
    }

    if (opt.from && opt.to && *opt.from > *opt.to) {
        std::cerr << "Invalid time range: --from is later than --to\n";
        return 2;
    }

    std::ifstream in(opt.input_path);
    if (!in) {
        std::cerr << "Cannot open input file: " << opt.input_path << "\n";
        return 2;
    }

    // Import
    auto import = pdt::read_csv(in);

    // DataSet
    pdt::DataSet ds{std::move(import.samples)};

    // Filter
    pdt::DataSet filtered = ds.filter({         // temporary FilterOptions object; 'opt' is a CliOptions object
        .sensor = opt.sensor,
        .from = opt.from,
        .to = opt.to
    });

    // Stats
    auto st = filtered.stats();

    // Output: import summary + filter summary + stats
    std::cout << "Import:\n";
    std::cout << "  parsed_ok: " << import.parsed_ok << "\n";
    std::cout << "  skipped:   " << import.skipped << "\n";
    std::cout << "Data:\n";
    std::cout << "  total:     " << ds.size() << "\n";
    std::cout << "  filtered:  " << filtered.size() << "\n";
    std::cout << "Stats:\n";
    std::cout << "  count:  " << st.count << "\n";
    std::cout << "  min:    " << st.min << "\n";
    std::cout << "  max:    " << st.max << "\n";
    std::cout << "  mean:   " << st.mean << "\n";
    std::cout << "  stddev: " << st.stddev << "\n";

    return 0;
}




























