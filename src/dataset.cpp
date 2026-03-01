#include "pdt/dataset.h"

namespace pdt {

DataSet::DataSet(std::vector<Sample> samples) : samples_(std::move(samples)) {}

std::span<const Sample> DataSet::samples() const noexcept { return samples_; }

std::size_t DataSet::size() const noexcept { return samples().size(); }

bool DataSet::empty() const noexcept { return samples_.empty(); }

DataSet DataSet::filter(const FilterOptions& opt) const {
    std::vector<Sample> out;
    out.reserve(samples_.size());

    for (const auto& s : samples_) {
        if (opt.sensor && s.sensor != *opt.sensor)  // *opt.sensor == opt.sensor.value() <-- std::optional<T>;
            continue;

        if (opt.from && s.timestamp < *opt.from)
            continue;

        if (opt.to && s.timestamp > *opt.to)
            continue;

        out.push_back(s);
    }

    return DataSet{std::move(out)};
}

Stats DataSet::stats() const {
    return compute_stats(samples_);
}

} // namespace pdt
