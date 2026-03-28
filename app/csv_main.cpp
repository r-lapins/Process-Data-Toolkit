#include "pdt/csv/csv_reader.h"
#include "pdt/csv/anomaly.h"
#include "pdt/csv/dataset.h"
#include "pdt/csv/output.h"
#include "csv_cli_args.h"

#include <iostream>
#include <fstream>
#include <optional>
#include <string>

using csv_app::CliOptions;
using csv_app::parse_args;
using csv_app::print_help;

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

    // validate CLI options
    {
        if (opt.filter.from && opt.filter.to && *opt.filter.from > *opt.filter.to) {
            std::cerr << "Invalid time range: --from is later than --to\n";
            return 2;
        }

        if (opt.per_sensor && opt.filter.sensor) {
            std::cerr << "Invalid arguments: --per-sensor cannot be used with --sensor\n";
            return 2;
        }

        if (opt.per_sensor && opt.output_marked_csv_path) {
            std::cerr << "Invalid arguments: --out-marked-csv cannot be used with --per-sensor\n";
            return 2;
        }
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
    pdt::DataSet ds = std::move(import.dataSet);

    // Filter
    pdt::DataSet filtered = ds.filter({.sensor = opt.per_sensor ? std::nullopt : opt.filter.sensor,
                                       .from   = opt.filter.from,
                                       .to     = opt.filter.to
    });

    pdt::ReportContext ctx{};
    ctx.parsed_ok           = import.parsed_ok;
    ctx.skipped             = import.skipped;
    ctx.total               = ds.size();
    ctx.filtered            = filtered.size();
    ctx.filter.sensor       = opt.filter.sensor;
    ctx.filter.from         = opt.filter.from;
    ctx.filter.to           = opt.filter.to;
    ctx.anomaly_threshold   = opt.anomaly_threshold;
    ctx.anomaly_method      = opt.anomaly_method;
    ctx.top_n               = opt.top;

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

    if (opt.per_sensor) {
        const auto st = filtered.stats_by_sensor();

        if (opt.anomaly_threshold) {
            per_sensor_anoms = pdt::detect_anomalies_per_sensor(filtered, opt.anomaly_method, *opt.anomaly_threshold, opt.top);
        }

        pdt::write_json_report(*out_stream, ctx, st, per_sensor_anoms);
    } else {
        const auto st = filtered.stats();

        if (opt.anomaly_threshold) {
            global_anoms = pdt::detect_anomalies_global(filtered, opt.anomaly_method, *opt.anomaly_threshold, opt.top );
        }

        // Export filtered CSV dataset with anomaly markers
        if (opt.output_marked_csv_path) {
            std::ofstream marked_file(*opt.output_marked_csv_path);
            if (!marked_file) {
                std::cerr << "Cannot open marked CSV file: " << *opt.output_marked_csv_path << "\n";
                return 2;
            }

            const auto anomalies = global_anoms ? global_anoms->top : std::vector<pdt::Anomaly>{};

            if (!pdt::write_csv_with_anomaly_markers(marked_file, filtered, anomalies)) {
                std::cerr << "Failed to write marked CSV: " << *opt.output_marked_csv_path << "\n";
                return 2;
            }

            std::cout << "[OK] Filtered CSV with marked anomalies saved to: " << *opt.output_marked_csv_path << "\n";

            if (!opt.output_path) { return 0; }
        }

        pdt::write_json_report(*out_stream, ctx, st, global_anoms);
    }

    if (opt.output_path) {
        std::cout << "[OK] Report saved to: " << *opt.output_path << "\n";
    }

    return 0;
}
