#include "pdt/stats.h"
#include <algorithm>
#include <cmath>
#include <numeric>

namespace pdt {

Stats compute_stats(std::span<const Sample> samples) {
    Stats result{};
    if (samples.empty())
        return result;

    std::vector<double> values;
    values.reserve(samples.size());

    for (const auto& s : samples)
        values.push_back(s.value);

    result.count = values.size();
    result.min = *std::min_element(values.begin(), values.end());
    result.max = *std::max_element(values.begin(), values.end());

    double sum = std::accumulate(values.begin(), values.end(), 0.0);
    result.mean = sum / values.size();

    double sq_sum = 0.0;
    for (double v : values)
        sq_sum += (v - result.mean) * (v - result.mean);

    result.stddev = std::sqrt(sq_sum / values.size());

    return result;
}

} // namespace pdt
