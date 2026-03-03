#include "pdt/dataset.h"
#include <algorithm>
#include <cmath>

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
    Stats result{};

    if (samples_.empty())
        return result;

    result.count = samples_.size();

    double sum = 0.0;
    result.min = samples_.front().value;
    result.max = samples_.front().value;

    for (const auto& s : samples_) {
        sum += s.value;
        result.min = std::min(result.min, s.value);
        result.max = std::max(result.max, s.value);
    }

    result.mean = sum / result.count;

    double sq_sum = 0.0;
    for (const auto& s : samples_) {
        double diff = s.value - result.mean;
        sq_sum += diff * diff;
    }

    result.stddev = std::sqrt(sq_sum / result.count);

    return result;
}

std::map<std::string, Stats> DataSet::stats_by_sensor() const {
    // grouping -> calculating statistics on groups
    std::map<std::string, std::vector<Sample>> groups;
    for (const auto& s : samples_) {
        groups[s.sensor].push_back(s);
    }

    std::map<std::string, Stats> out;
    for (auto& [sensor, vec] : groups) {
        DataSet ds{std::move(vec)};
        out.emplace(sensor, ds.stats());
    }

    return out;
}

} // namespace pdt
