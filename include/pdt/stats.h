#ifndef STATS_H
#define STATS_H

#pragma once
#include "types.h"
#include <span>

namespace pdt {

Stats compute_stats(std::span<const Sample> samples);

} // namespace pdt


// class stats
// {
// public:
//     stats();
// };

#endif // STATS_H
