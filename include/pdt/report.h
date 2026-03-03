#pragma once
#include "dataset.h"
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
};

void write_json_report(std::ostream& os,
                       const ReportContext& ctx,
                       const Stats& stats);

void write_json_report(std::ostream& os,
                       const ReportContext& ctx,
                       const std::map<std::string, Stats>& per_sensor);

} // namespace pdt
