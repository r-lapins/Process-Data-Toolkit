#include "pdt/core/csv_reader.h"
#include "pdt/core/time.h"

#include <sstream>

// public library namespace
namespace pdt {

// anonynmous namespace (internal linkage)
namespace {

bool parse_line(const std::string& line, Sample& out_sample) {
    std::istringstream ss(line);
    std::string ts_str;
    std::string sensor;
    std::string value_str;

    if (!std::getline(ss, ts_str, ',')) {
        return false;
    }

    if (!std::getline(ss, sensor, ',')) {
        return false;
    }

    if (!std::getline(ss, value_str, ',')) {
        return false;
    }

    try {
        auto ts = parse_iso8601(ts_str);
        if (!ts) {
            return false;
        }

        out_sample.timestamp = *ts;
        out_sample.sensor = sensor;
        out_sample.value = std::stod(value_str);    // string to double
    } catch (const std::exception&) {
        return false;
    }

    return true;
}

} // namespace

ImportResult read_csv(std::istream& input) {
    ImportResult result;
    std::string line;
    std::size_t line_number = 0;

    // skip header
    if (!std::getline(input, line)) {
        return result;
    }

    ++line_number;

    while (std::getline(input, line)) {
        ++line_number;

        if (line.empty()) {
            continue;
        }

        Sample sample{};
        if (parse_line(line, sample)) {
            result.samples.push_back(std::move(sample));
            ++result.parsed_ok;
        } else {
            ++result.skipped;
            result.skipped_rows.push_back(SkippedRow{
                .line_number = line_number,
                .text = line
            });
        }
    }

    return result;
}

} // namespace pdt

