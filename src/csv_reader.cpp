#include "pdt/csv_reader.h"
#include <sstream>

namespace pdt {

std::vector<Sample> read_csv(std::istream& input) {
    std::vector<Sample> result;
    std::string line;

    while (std::getline(input, line)) {
        std::istringstream ss(line);
        std::string ts_str, sensor, value_str;

        if (!std::getline(ss, ts_str, ','))
            continue;
        if (!std::getline(ss, sensor, ','))
            continue;
        if (!std::getline(ss, value_str, ','))
            continue;

        Sample s{};
        s.timestamp = std::chrono::sys_seconds{};
        s.sensor = sensor;
        s.value = std::stod(value_str);

        result.push_back(std::move(s));
    }

    return result;
}

} // namespace pdt


// csv_reader::csv_reader() {}
