#include "pdt/core/csv_reader.h"
#include "pdt/core/anomaly.h"
#include "pdt/core/dataset.h"
#include "pdt/core/output.h"
#include "core_cli_args.h"

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

    if (opt.per_sensor && opt.sensor) {
        std::cerr << "Invalid arguments: --per-sensor cannot be used with --sensor\n";
        return 2;
    }

    std::ifstream in(opt.input_path);
    if (!in) {
        std::cerr << "Cannot open input file: " << opt.input_path << "\n";
        return 2;
    }

    // Import
    auto import = pdt::read_csv(in);

    if (opt.show_skipped && !import.skipped_rows.empty()) {
        std::cerr << "Skipped CSV rows: " << import.skipped_rows.size() << "\n";

        for (const auto& row : import.skipped_rows) {
            std::cerr << "line " << row.line_number << ": " << row.text << "\n";
        }

        return 0;
    }

    // DataSet
    pdt::DataSet ds{std::move(import.samples)};

    // Filter
    pdt::DataSet filtered = ds.filter({         // temporary FilterOptions object; 'opt' is a CliOptions object
        .sensor = opt.per_sensor ? std::nullopt : opt.sensor,   // in case both are delivered
        .from = opt.from,
        .to = opt.to
    });

    pdt::ReportContext ctx{};
    ctx.parsed_ok = import.parsed_ok;
    ctx.skipped = import.skipped;
    ctx.total = ds.size();
    ctx.filtered = filtered.size();
    ctx.sensor = opt.sensor;
    ctx.from = opt.from;
    ctx.to = opt.to;
    ctx.anomaly_threshold = opt.anomaly_threshold;
    ctx.anomaly_method = opt.anomaly_method;
    ctx.top_n = opt.top;

    // output
    std::ostream* out_stream = &std::cout;
    std::ofstream file;

    if (opt.output_path) {
        file.open(*opt.output_path);
        if (!file) {
            std::cerr << "Cannot open  output file: " << *opt.output_path << "\n";
            return 2;
        }
        out_stream = &file;
    }
    // ~output out_stream -> file or stdout

    std::optional<pdt::AnomalySummary> global_anoms;
    std::optional<std::map<std::string, pdt::AnomalySummary>> per_sensor_anoms;

    // Stats & Anomalies & JSON report (function 'write_json_report' is overloaded)
    if (opt.per_sensor) {
        auto st = filtered.stats_by_sensor();

        if (opt.anomaly_threshold) { per_sensor_anoms = pdt::detect_anomalies_per_sensor(filtered, opt.anomaly_method, *opt.anomaly_threshold, opt.top); }

        pdt::write_json_report(*out_stream, ctx, st, per_sensor_anoms);
    } else {
        auto st = filtered.stats();

        if (opt.anomaly_threshold) { global_anoms = pdt::detect_anomalies_global(filtered, opt.anomaly_method, *opt.anomaly_threshold, opt.top); }

        pdt::write_json_report(*out_stream, ctx, st, global_anoms);
    }

    return 0;
}
