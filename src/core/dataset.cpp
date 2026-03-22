#include "pdt/core/dataset.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <numeric>
#include <vector>
#include <cassert>

namespace pdt {
namespace {

double quantile_sorted(std::span<const double> sorted_values, double q) {
    // debug build
    assert(q >= 0.0 && q <= 1.0);

    if (sorted_values.empty()) {
        return 0.0;
    }

    if (sorted_values.size() == 1) {
        return sorted_values.front();
    }

    const double position = q * static_cast<double>(sorted_values.size() - 1);
    const auto lower_index = static_cast<std::size_t>(std::floor(position));
    const auto upper_index = static_cast<std::size_t>(std::ceil(position));

    if (lower_index == upper_index) {
        return sorted_values[lower_index];
    }

    const double fraction = position - static_cast<double>(lower_index);
    const double lower = sorted_values[lower_index];
    const double upper = sorted_values[upper_index];

    return lower + (fraction * (upper - lower));
}

Stats compute_stats_from_samples(std::span<const Sample> samples) {
    Stats result{};

    if (samples.empty()) {
        return result;
    }

    std::vector<double> values;
    values.reserve(samples.size());

    for (const auto& s : samples) {
        values.push_back(s.value);
    }

    result.count = values.size();

    const auto [min_it, max_it] = std::ranges::minmax_element(values);
    result.min = *min_it;
    result.max = *max_it;

    const double sum = std::accumulate(values.begin(), values.end(), 0.0);
    result.mean = sum / static_cast<double>(values.size());

    double sq_sum = 0.0;
    for (double v : values) {
        const double d = v - result.mean;
        sq_sum += d * d;
    }

    result.stddev = std::sqrt(sq_sum / static_cast<double>(values.size()));

    std::ranges::sort(values);

    result.q1 = quantile_sorted(values, 0.25);
    result.median = quantile_sorted(values, 0.50);
    result.q3 = quantile_sorted(values, 0.75);

    return result;
}

bool matches_filter(const Sample& s, const FilterOptions& opt) {
    if (opt.sensor && s.sensor != *opt.sensor) { return false; }

    if (opt.from && s.timestamp < *opt.from) { return false; }

    if (opt.to && s.timestamp > *opt.to) { return false; }

    return true;
}

} // namespace

DataSet::DataSet(std::vector<Sample> samples) : samples_(std::move(samples)) {}

std::span<const Sample> DataSet::samples() const noexcept { return samples_; }

std::size_t DataSet::size() const noexcept { return samples_.size(); }

bool DataSet::empty() const noexcept { return samples_.empty(); }

DataSet DataSet::filter(const FilterOptions& opt) const {
    std::vector<Sample> filtered;
    filtered.reserve(samples_.size());

    for (const auto& s : samples_) {
        if (matches_filter(s, opt)) {
            filtered.push_back(s);
        }
    }

    return DataSet{std::move(filtered)};
}

Stats DataSet::stats() const {
    return compute_stats_from_samples(samples_);
}

std::map<std::string, Stats> DataSet::stats_by_sensor() const {
    // grouping -> calculating statistics on groups
    std::map<std::string, std::vector<Sample>> grouped;

    for (const auto& s : samples_) {
        grouped[s.sensor].push_back(s);
    }

    std::map<std::string, Stats> result;
    for (const auto& [sensor, sensor_samples] : grouped) {
        result.emplace(sensor, compute_stats_from_samples(sensor_samples));
    }

    return result;
}

} // namespace pdt
