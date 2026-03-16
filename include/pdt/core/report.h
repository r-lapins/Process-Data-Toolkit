#pragma once

#include "anomaly.h"

#include <ostream>
#include <string>

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
    std::optional<double> z_threshold;
    std::size_t top_n{10};
};

void write_json_report(std::ostream& os,
                       const ReportContext& ctx,
                       const Stats& stats,
                       const std::optional<AnomalySummary>& global_anomalies);

void write_json_report(std::ostream& os,
                       const ReportContext& ctx,
                       const std::map<std::string, Stats>& per_sensor,
                       const std::optional<std::map<std::string, AnomalySummary>>& per_sensor_anomalies);

} // namespace pdt
