#include "pdt/core/report.h"

#include <chrono>
#include <iomanip>
#include <map>
#include <optional>
#include <ostream>
#include <sstream>
#include <string>

namespace {

std::string anomaly_method_to_string(pdt::AnomalyMethod method) {
    using enum pdt::AnomalyMethod;
    switch (method) {
    case ZScore: return "zscore";
    case IQR:    return "iqr";
    case MAD:    return "mad";
    }

    return "unknown";
}

std::string to_string(std::chrono::sys_seconds ts) {
    using namespace std::chrono;

    const auto dp = floor<days>(ts);
    const year_month_day ymd{dp};
    const auto time = hh_mm_ss{ts - dp};

    std::ostringstream oss;
    oss << std::setw(4) << std::setfill('0') << int(ymd.year()) << "-"
        << std::setw(2) << unsigned(ymd.month()) << "-"
        << std::setw(2) << unsigned(ymd.day()) << "T"
        << std::setw(2) << time.hours().count() << ":"
        << std::setw(2) << time.minutes().count() << ":"
        << std::setw(2) << time.seconds().count();

    return oss.str();
}

void write_anomaly(std::ostream& os, const pdt::Anomaly& a, int indent) {
    const std::string sp(indent, ' ');

    os << sp << "{"
       << "\"timestamp\":\"" << to_string(a.timestamp) << "\","
       << "\"sensor\":\"" << a.sensor << "\","
       << "\"value\":" << a.value << ","
       << "\"score\":" << a.score
       << "}";
}

void write_anomaly_summary(std::ostream& os, const pdt::AnomalySummary& s, int indent) {
    const std::string sp(indent, ' ');

    if (s.top.empty()) {
        os << "{ \"count\": " << s.count << ", \"top\": [] }";
        return;
    }

    os << "{\n";
    os << sp << "  \"count\": " << s.count << ",\n";
    os << sp << "  \"top\": [\n";

    for (std::size_t i = 0; i < s.top.size(); ++i) {
        write_anomaly(os, s.top[i], indent + 4);
        os << (i + 1 < s.top.size() ? ",\n" : "\n");
    }

    os << sp << "  ]\n";
    os << sp << "}";
}

} // namespace

namespace pdt {

void write_json_report(std::ostream& os,
                       const ReportContext& ctx,
                       const Stats& stats,
                       const std::optional<AnomalySummary>& global_anomalies) {
    os << "{\n";

    os << "  \"mode\": \"" << (ctx.sensor ? "sensor" : "global") << "\",\n";

    os << "  \"import\": {\n";
    os << "    \"parsed_ok\": " << ctx.parsed_ok << ",\n";
    os << "    \"skipped\": " << ctx.skipped << "\n";
    os << "  },\n";

    os << "  \"filter\": {\n";
    bool first = true;

    const auto field = [&](const char* key, const std::string& value) {
        if (!first) { os << ",\n"; }
        first = false;
        os << "    \"" << key << "\": \"" << value << "\"";
    };

    if (ctx.sensor) { field("sensor", *ctx.sensor); }
    if (ctx.from) { field("from", to_string(*ctx.from)); }
    if (ctx.to) { field("to", to_string(*ctx.to)); }

    if (!first) { os << '\n'; }
    os << "  },\n";

    os << "  \"data\": {\n";
    os << "    \"total\": " << ctx.total << ",\n";
    os << "    \"filtered\": " << ctx.filtered << "\n";
    os << "  },\n";

    os << "  \"stats\": {\n";
    os << "    \"count\": " << stats.count << ",\n";
    os << "    \"min\": " << stats.min << ",\n";
    os << "    \"max\": " << stats.max << ",\n";
    os << "    \"mean\": " << stats.mean << ",\n";
    os << "    \"stddev\": " << stats.stddev << "\n";
    os << "  }";

    if (ctx.anomaly_threshold && ctx.anomaly_method) {
        os << ",\n";
        os << "  \"anomalies\": {\n";
        os << "    \"method\": \"" << anomaly_method_to_string(*ctx.anomaly_method) << "\",\n";
        os << "    \"threshold\": " << *ctx.anomaly_threshold << ",\n";
        os << "    \"top_n\": " << ctx.top_n << ",\n";
        os << "    \"mode\": \"global\",\n";
        os << "    \"global\": ";

        if (global_anomalies) {
            write_anomaly_summary(os, *global_anomalies, 4);
            os << '\n';
        }
        else { os << "{ \"count\": 0, \"top\": [] }\n"; }

        os << "  }\n";
    }
    else { os << '\n'; }

    os << "}\n";
}

void write_json_report(std::ostream& os,
                       const ReportContext& ctx,
                       const std::map<std::string, Stats>& per_sensor,
                       const std::optional<std::map<std::string, AnomalySummary>>& per_sensor_anomalies) {
    os << "{\n";

    os << "  \"mode\": \"per_sensor\",\n";

    os << "  \"import\": {\n";
    os << "    \"parsed_ok\": " << ctx.parsed_ok << ",\n";
    os << "    \"skipped\": " << ctx.skipped << "\n";
    os << "  },\n";

    os << "  \"filter\": {\n";
    bool first = true;

    const auto field = [&](const char* key, const std::string& value) {
        if (!first) { os << ",\n"; }
        first = false;
        os << "    \"" << key << "\": \"" << value << "\"";
    };

    if (ctx.from) { field("from", to_string(*ctx.from)); }
    if (ctx.to) { field("to", to_string(*ctx.to)); }

    if (!first) { os << '\n'; }
    os << "  },\n";

    os << "  \"data\": {\n";
    os << "    \"total\": " << ctx.total << ",\n";
    os << "    \"filtered\": " << ctx.filtered << "\n";
    os << "  },\n";

    os << "  \"stats_by_sensor\": {\n";

    bool first_sensor = true;
    for (const auto& [name, st] : per_sensor) {
        if (!first_sensor) { os << ",\n"; }
        first_sensor = false;

        os << "    \"" << name << "\": {\n";
        os << "      \"count\": " << st.count << ",\n";
        os << "      \"min\": " << st.min << ",\n";
        os << "      \"max\": " << st.max << ",\n";
        os << "      \"mean\": " << st.mean << ",\n";
        os << "      \"stddev\": " << st.stddev << "\n";
        os << "    }";
    }

    os << "\n  }";

    if (ctx.anomaly_threshold && ctx.anomaly_method) {
        os << ",\n";
        os << "  \"anomalies\": {\n";
        os << "    \"method\": \"" << anomaly_method_to_string(*ctx.anomaly_method) << "\",\n";
        os << "    \"threshold\": " << *ctx.anomaly_threshold << ",\n";
        os << "    \"top_n\": " << ctx.top_n << ",\n";
        os << "    \"mode\": \"per_sensor\",\n";
        os << "    \"per_sensor\": {\n";

        if (per_sensor_anomalies && !per_sensor_anomalies->empty()) {
            std::size_t i = 0;
            for (const auto& [sensor, summary] : *per_sensor_anomalies) {
                os << "      \"" << sensor << "\": ";
                write_anomaly_summary(os, summary, 6);
                os << (++i < per_sensor_anomalies->size() ? ",\n" : "\n");
            }
        }

        os << "    }\n";
        os << "  }\n";
    }
    else { os << '\n'; }

    os << "}\n";
}

} // namespace pdt