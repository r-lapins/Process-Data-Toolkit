#pragma once

#include "anomaly.h"

#include <ostream>
#include <string>
#include <cstddef>
#include <ostream>
#include <span>

namespace pdt {

struct ReportContext {
    std::size_t parsed_ok{};
    std::size_t skipped{};
    std::size_t total{};
    std::size_t filtered{};

    std::optional<std::string> sensor;
    std::optional<std::chrono::sys_seconds> from;
    std::optional<std::chrono::sys_seconds> to;

    // anomaly settings
    std::optional<double> anomaly_threshold;
    std::optional<AnomalyMethod> anomaly_method;
    std::size_t top_n{10};
};

std::string format_anomaly_line(const Anomaly& anomaly, std::size_t displayIndex, AnomalyMethod method);

void write_json_report(std::ostream& os, const ReportContext& ctx, const Stats& stats,
                       const std::optional<AnomalySummary>& global_anomalies);

void write_json_report(std::ostream& os, const ReportContext& ctx, const std::map<std::string, Stats>& perSensor,
                       const std::optional<std::map<std::string, AnomalySummary>>& perSensorAnomalies);

bool write_csv(std::ostream& os, const DataSet& dataSet);

// saves the dataset and, after each row that is an anomaly from the `anomalies` list,
// adds an extra line: anomaly;
bool write_csv_with_anomaly_markers(std::ostream& os, const DataSet& dataSet, std::span<const Anomaly> anomalies);

} // namespace pdt
