#pragma once
#include "types.h"
#include <span>

namespace pdt {

Stats compute_stats(std::span<const Sample> samples);

} // namespace pdt
