#pragma once
#include "stats.h"
#include "types.h"

#include <span>
#include <vector>

// a collection of process data that can be filtered and used to calculate statistics

namespace pdt {

class DataSet {
  public:
    DataSet() = default;
    explicit DataSet(std::vector<Sample> samples);

    // read-only access
    std::span<const Sample> samples() const noexcept;

    std::size_t size() const noexcept;
    bool empty() const noexcept;

    auto begin() const noexcept { return samples_.begin(); }
    auto end() const noexcept { return samples_.end(); }

    //domains ops
    DataSet filter(const FilterOptions& opt) const;
    Stats stats() const;

  private:
    std::vector<Sample> samples_;
};

} // namespace pdt
