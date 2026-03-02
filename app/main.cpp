#include "pdt/dataset.h"
#include "pdt/csv_reader.h"
#include "cli_args.h"

#include <iostream>
#include <fstream>
#include <optional>
#include <string>

using pdt_app::CliOptions;
using pdt_app::parse_args;
using pdt_app::print_help;

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




























