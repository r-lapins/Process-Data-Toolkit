#include "pdt/filter.h"

namespace pdt {

std::vector<Sample> filter_samples(const std::vector<Sample>& samples, const FilterOptions& opt) {
    std::vector<Sample> out;
    out.reserve(samples.size());

    for (const auto& s : samples) {
        if (opt.sensor && s.sensor != *opt.sensor)
            continue;

        if (opt.from && s.timestamp < *opt.from)
            continue;

        if (opt.to && s.timestamp > *opt.to)
            continue;

        out.push_back(s);
    }

    return out;
}

} // namespace pdt
