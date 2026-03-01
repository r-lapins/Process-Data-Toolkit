#include "pdt/dataset.h"

namespace pdt {

DataSet::DataSet(std::vector<Sample> samples) : samples_(std::move(samples)) {}

std::span<const Sample> DataSet::samples() const noexcept { return samples_; }

std::size_t DataSet::size() const noexcept { return samples().size(); }

bool DataSet::empty() const noexcept { return samples_.empty(); }

DataSet DataSet::filter(const FilterOptions& opt) const {
    return DataSet{filter_samples(samples_, opt)};
}

Stats DataSet::stats() const {
    return compute_stats(samples_);
}

} // namespace pdt
