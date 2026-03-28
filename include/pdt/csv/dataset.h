#pragma once

#include "types.h"

#include <map>
#include <span>
#include <vector>
#include <cstddef>
#include <string>

// a collection of process data that can be filtered and used to calculate statistics

namespace pdt {

// free function
[[nodiscard]] Stats compute_stats(std::span<const Sample> samples);

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
    std::map<std::string, Stats> stats_by_sensor() const;
    std::map<std::string, DataSet> split_by_sensor() const;

  private:
    std::vector<Sample> samples_;
};

} // namespace pdt
