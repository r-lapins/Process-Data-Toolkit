#pragma once

#include "types.h"

#include <istream>
#include <vector>
#include <cstddef>
#include <string>

namespace pdt {

struct SkippedRow {
    std::size_t line_number{};
    std::string text;
};

struct CsvData {
    std::vector<Sample> samples;
    std::size_t parsed_ok{};
    std::size_t skipped{};
    std::vector<SkippedRow> skipped_rows;
};

CsvData read_csv(std::istream& input);

} // namespace pdt
