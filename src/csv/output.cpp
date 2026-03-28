#include "pdt/csv/output.h"
#include "pdt/csv/time.h"

#include <optional>
#include <iomanip>
#include <chrono>
#include <ostream>
#include <sstream>
#include <string>
#include <map>
#include <set>

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

void write_anomaly(std::ostream& os, const pdt::Anomaly& a, int indent) {
    const std::string sp(indent, ' ');

    os << sp << "{"
       << "\"timestamp\":\""    << pdt::format_iso8601(a.timestamp) << "\","
       << "\"sensor\":\""       << a.sensor << "\","
       << "\"value\":"          << a.value << ","
       << "\"score\":"          << a.score
       << "}";
}

void write_anomaly_summary(std::ostream& os, const pdt::AnomalySummary& s, int indent) {
    const std::string sp(indent, ' ');

    if (s.top.empty()) {
        os << "{ \"count\": " << s.all.size() << ", \"top\": [] }";
        return;
    }

    os << "{\n";
    os << sp << "  \"count\": " << s.all.size() << ",\n";
    os << sp << "  \"top\": [\n";

    for (std::size_t i = 0; i < s.top.size(); ++i) {
        write_anomaly(os, s.top[i], indent + 4);
        os << (i + 1 < s.top.size() ? ",\n" : "\n");
    }

    os << sp << "  ]\n";
    os << sp << "}";
}

std::string anomaly_score_label(pdt::AnomalyMethod method) {
    using enum pdt::AnomalyMethod;

    switch (method) {
    case ZScore: return "z";
    case IQR:    return "iqr";
    case MAD:    return "mad";
    }

    return "score";
}

} // namespace

namespace pdt {

void write_json_report(std::ostream& os, const ReportContext& ctx, const Stats& stats,
                       const std::optional<AnomalySummary>& globalAnomalies) {
    os << "{\n";

    os << "  \"mode\": \""      << (ctx.filter.sensor? "sensor" : "global") << "\",\n";

    os << "  \"import\": {\n";
    os << "    \"parsed_ok\": " << ctx.parsed_ok << ",\n";
    os << "    \"skipped\": "   << ctx.skipped << "\n";
    os << "  },\n";

    os << "  \"filter\": {\n";
    bool first = true;

    const auto field = [&](const char* key, const std::string& value) {
        if (!first) { os << ",\n"; }
        first = false;
        os << "    \"" << key << "\": \"" << value << "\"";
    };

    if (ctx.filter.sensor)  { field("sensor", *ctx.filter.sensor); }
    if (ctx.filter.from)    { field("from", format_iso8601(*ctx.filter.from)); }
    if (ctx.filter.to)      { field("to", format_iso8601(*ctx.filter.to)); }

    if (!first) { os << '\n'; }
    os << "  },\n";

    os << "  \"data\": {\n";
    os << "    \"total\": "     << ctx.total << ",\n";
    os << "    \"filtered\": "  << ctx.filtered << "\n";
    os << "  },\n";

    os << "  \"stats\": {\n";
    os << "    \"count\": "     << stats.count << ",\n";
    os << "    \"min\": "       << stats.min << ",\n";
    os << "    \"max\": "       << stats.max << ",\n";
    os << "    \"mean\": "      << stats.mean << ",\n";
    os << "    \"stddev\": "    << stats.stddev << "\n";
    os << "  }";

    if (ctx.anomaly_threshold && ctx.anomaly_method) {
        os << ",\n";
        os << "  \"anomalies\": {\n";
        os << "    \"method\": \""  << anomaly_method_to_string(*ctx.anomaly_method) << "\",\n";
        os << "    \"threshold\": " << *ctx.anomaly_threshold << ",\n";
        os << "    \"top_n\": "     << ctx.top_n << ",\n";
        os << "    \"mode\": \"global\",\n";
        os << "    \"global\": ";

        if (globalAnomalies) {
            write_anomaly_summary(os, *globalAnomalies, 4);
            os << '\n';
        }
        else { os << "{ \"count\": 0, \"top\": [] }\n"; }

        os << "  }\n";
    }
    else { os << '\n'; }

    os << "}\n";
}

void write_json_report(std::ostream& os, const ReportContext& ctx, const std::map<std::string, Stats>& perSensor,
                       const std::optional<std::map<std::string, AnomalySummary>>& perSensorAnomalies) {
    os << "{\n";

    os << "  \"mode\": \"per_sensor\",\n";

    os << "  \"import\": {\n";
    os << "    \"parsed_ok\": " << ctx.parsed_ok << ",\n";
    os << "    \"skipped\": "   << ctx.skipped << "\n";
    os << "  },\n";

    os << "  \"filter\": {\n";
    bool first = true;

    const auto field = [&](const char* key, const std::string& value) {
        if (!first) { os << ",\n"; }
        first = false;
        os << "    \"" << key << "\": \"" << value << "\"";
    };

    if (ctx.filter.from)    { field("from", format_iso8601(*ctx.filter.from)); }
    if (ctx.filter.to)      { field("to", format_iso8601(*ctx.filter.to)); }

    if (!first) { os << '\n'; }
    os << "  },\n";

    os << "  \"data\": {\n";
    os << "    \"total\": "     << ctx.total << ",\n";
    os << "    \"filtered\": "  << ctx.filtered << "\n";
    os << "  },\n";

    os << "  \"stats_by_sensor\": {\n";

    bool firstSensor = true;
    for (const auto& [name, st] : perSensor) {
        if (!firstSensor) { os << ",\n"; }
        firstSensor = false;

        os << "    \""              << name << "\": {\n";
        os << "      \"count\": "   << st.count << ",\n";
        os << "      \"min\": "     << st.min << ",\n";
        os << "      \"max\": "     << st.max << ",\n";
        os << "      \"mean\": "    << st.mean << ",\n";
        os << "      \"stddev\": "  << st.stddev << "\n";
        os << "    }";
    }

    os << "\n  }";

    if (ctx.anomaly_threshold && ctx.anomaly_method) {
        os << ",\n";
        os << "  \"anomalies\": {\n";
        os << "    \"method\": \""  << anomaly_method_to_string(*ctx.anomaly_method) << "\",\n";
        os << "    \"threshold\": " << *ctx.anomaly_threshold << ",\n";
        os << "    \"top_n\": "     << ctx.top_n << ",\n";
        os << "    \"mode\": \"per_sensor\",\n";
        os << "    \"per_sensor\": {\n";

        if (perSensorAnomalies && !perSensorAnomalies->empty()) {
            std::size_t i = 0;
            for (const auto& [sensor, summary] : *perSensorAnomalies) {
                os << "      \""    << sensor << "\": ";
                write_anomaly_summary(os, summary, 6);
                os << (++i < perSensorAnomalies->size() ? ",\n" : "\n");
            }
        }

        os << "    }\n";
        os << "  }\n";
    }
    else { os << '\n'; }

    os << "}\n";
}

std::string format_anomaly_line(const Anomaly& anomaly, std::size_t displayIndex, AnomalyMethod method)
{
    std::ostringstream oss;

    oss << std::fixed << std::setprecision(2);

    oss << std::setw(3) << std::setfill(' ') << displayIndex << ". ";
    oss << "  |  "          << format_date_time(anomaly.timestamp)
        << "  |  "          << anomaly.sensor
        << "  |  value = "  << anomaly.value
        << "  |  "          << anomaly_score_label(method)
        << " = "            << anomaly.score;

    return oss.str();
}

bool write_csv(std::ostream &os, const DataSet &dataSet)
{
    if (!os) { return false; }

    os << "timestamp,sensor,value\n";

    for (const auto& sample : dataSet.samples()) {
        os << format_iso8601(sample.timestamp) << ','
           << sample.sensor << ','
           << sample.value << '\n';
    }

    return static_cast<bool>(os);
}

bool write_csv_with_anomaly_markers(std::ostream &os, const DataSet &dataSet, std::span<const Anomaly> anomalies)
{
    if (!os) { return false; }

    os << "timestamp,sensor,value\n";

    std::set<std::size_t> anomalyIndices;
    for (const auto& anomaly : anomalies) { anomalyIndices.insert(anomaly.index); }

    const auto samples = dataSet.samples();
    for (std::size_t i = 0; i < samples.size(); ++i) {
        const auto& sample = samples[i];

        os << format_iso8601(sample.timestamp) << ','
           << sample.sensor << ','
           << sample.value << '\n';

        if (anomalyIndices.contains(i)) {
            os << "anomaly\n";
        }
    }

    return static_cast<bool>(os);
}

} // namespace pdt