#pragma once

#include "dataset.h"

#include <chrono>
#include <map>
#include <string>
#include <vector>
#include <cstddef>

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
    std::size_t index{}; // index in original dataset (or filtered view)
};

struct AnomalySummary {
    std::vector<Anomaly> all;
    std::vector<Anomaly> top; // sorted descending by |score|
};

std::vector<Anomaly> select_top_anomalies(std::span<const Anomaly> anomalies, std::size_t max_count);

// unified API
AnomalySummary detect_anomalies_global(const DataSet& ds, AnomalyMethod method, double threshold, std::size_t top_n);
std::map<std::string, AnomalySummary> detect_anomalies_per_sensor(const DataSet& ds, AnomalyMethod method, double threshold, std::size_t top_n);

} // namespace pdt


// // Z-score
// AnomalySummary detect_zscore_global(const DataSet& ds, double threshold, std::size_t top_n);
// std::map<std::string, AnomalySummary> detect_zscore_per_sensor(const DataSet& ds, double threshold, std::size_t top_n);

// // IQR
// AnomalySummary detect_iqr_global(const DataSet& ds, double threshold, std::size_t top_n);
// std::map<std::string, AnomalySummary> detect_iqr_per_sensor(const DataSet& ds, double threshold, std::size_t top_n);

// // MAD
// AnomalySummary detect_mad_global(const DataSet& ds, double threshold, std::size_t top_n);
// std::map<std::string, AnomalySummary> detect_mad_per_sensor(const DataSet& ds, double threshold, std::size_t top_n);
