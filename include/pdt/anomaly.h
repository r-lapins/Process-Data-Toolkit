#pragma once
#include "dataset.h"

namespace pdt {

struct Anomaly {
    std::chrono::sys_seconds timestamp;
    std::string sensor;
    double value{};
    double zscore{};
};

struct AnomalySummary {
    std::size_t count{};
    std::vector<Anomaly> top; // posortowane malejąco po |z|
};

// global mode
AnomalySummary detect_zscore_global(const DataSet& ds, double threshold,
                                    std::size_t top_n);

//per-sensor mode
std::map<std::string, AnomalySummary> detect_zscore_per_sensor(const DataSet& ds, double threshold,
                                                               std::size_t top_n);

} // namespace pdt
