#include "pdt/csv_reader.h"
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

    if (!std::getline(ss, ts_str, ','))
        return false;

    if (!std::getline(ss, sensor, ','))
        return false;

    if (!std::getline(ss, value_str, ','))
        return false;

    try {
        out_sample.timestamp = std::chrono::sys_seconds{}; // na razie placeholder
        out_sample.sensor = sensor;
        out_sample.value = std::stod(value_str);    // string to double
    } catch (...) {
        return false;
    }

    return true;
}

} // namespace

ImportResult read_csv(std::istream& input) {
    ImportResult result;
    std::string line;

    if (!std::getline(input, line))
        return result;

    while (std::getline(input, line)) {
        if (line.empty())
            continue;

        Sample sample{};
        if (parse_line(line, sample)) {
            result.samples.push_back(std::move(sample));
            ++result.parsed_ok;
        } else {
            ++result.skipped;
        }
    }

    return result;
}

} // namespace pdt

