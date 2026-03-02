#include "pdt/csv_reader.h"
#include "pdt/dataset.h"
#include "pdt/report.h"
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

    pdt::ReportContext ctx{};
    ctx.parsed_ok = import.parsed_ok;
    ctx.skipped = import.skipped;
    ctx.total = ds.size();
    ctx.filtered = filtered.size();
    ctx.sensor = opt.sensor;
    ctx.from = opt.from;
    ctx.to = opt.to;

    if (opt.output_path) {
        std::ofstream out(*opt.output_path);
        if (!out) {
            std::cerr << "Cannot open  output file: " << *opt.output_path << "\n";
            return 2;
        }
        pdt::write_json_report(out, ctx, st);
    } else {
        pdt::write_json_report(std::cout, ctx, st);
    }

    return 0;
}





























