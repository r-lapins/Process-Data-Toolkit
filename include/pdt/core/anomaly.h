#pragma once

#include "dataset.h"

namespace pdt {

enum class AnomalyMethod {
    ZScore,
    IQR,
    MAD
};

struct Anomaly {
    std::chrono::sys_seconds timestamp;
    std::string sensor;
    double value{};
    double score{}; // generic anomaly score
};

struct AnomalySummary {
    std::size_t count{};
    std::vector<Anomaly> top; // sorted descending by |score|
};

// unified API: per-sensor mode
std::map<std::string, AnomalySummary> detect_anomalies_per_sensor(const DataSet& ds,
                                                                  AnomalyMethod method,
                                                                  double threshold,
                                                                  std::size_t top_n);

// temporary public helpers
AnomalySummary detect_zscore_global(const DataSet& ds,
                                    double threshold,
                                    std::size_t top_n);

std::map<std::string, AnomalySummary> detect_zscore_per_sensor(const DataSet& ds,
                                                               double threshold,
                                                               std::size_t top_n);


} // namespace pdt
