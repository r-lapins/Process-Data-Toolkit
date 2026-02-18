#ifndef CSV_READER_H
#define CSV_READER_H

#pragma once
#include "types.h"
#include <istream>
#include <vector>

namespace pdt {

std::vector<Sample> read_csv(std::istream& input);

} // namespace pdt


// class csv_reader
// {
// public:
//     csv_reader();
// };

#endif // CSV_READER_H
