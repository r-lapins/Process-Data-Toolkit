#pragma once
#include "filter.h"
#include "stats.h"
#include "types.h"

#include <span>
#include <vector>

namespace pdt {

class DataSet {
  public:
    DataSet() = default;
    explicit DataSet(std::vector<Sample> samples);

    // read-only access
    std::span<const Sample> samples() const noexcept;

    std::size_t size() const noexcept;
    bool empty() const noexcept;

    //domains ops
    DataSet filter(const FilterOptions& opt) const;
    Stats stats() const;

  private:
    std::vector<Sample> samples_;
};

} // namespace pdt
