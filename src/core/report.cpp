#include "pdt/core/report.h"

namespace {

std::string to_string(std::chrono::sys_seconds ts) {
    using namespace std::chrono;
    auto dp = floor<days>(ts);
    year_month_day ymd{dp};

    auto time = hh_mm_ss{ts - dp};

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
    std::string sp(indent, ' ');
    os << sp << "{"
       << "\"timestamp\":\"" << to_string(a.timestamp) << "\","
       << "\"sensor\":\"" << a.sensor << "\","
       << "\"value\":" << a.value << ","
       << "\"z\":" << a.zscore
       << "}";
}

void write_anomaly_summary(std::ostream& os, const pdt::AnomalySummary& s, int indent) {
    std::string sp(indent, ' ');

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
                       const std::optional<AnomalySummary>& global_anomalies)
{
    os << "{\n";

    os << "  \"mode\": \"" << (ctx.sensor ? "sensor" : "global") << "\",\n";

    os << "  \"import\": {\n";
    os << "    \"parsed_ok\": " << ctx.parsed_ok << ",\n";
    os << "    \"skipped\": " << ctx.skipped << "\n";
    os << "  },\n";

    os << "  \"filter\": {\n";
    bool first = true;

    auto field = [&](const char* k, const std::string& v) {
        if (!first) {os << ",\n";}
        first = false;
        os << "    \"" << k << "\": \"" << v << "\"";
    };

    if (ctx.sensor) {field("sensor", *ctx.sensor);}
    if (ctx.from)   {field("from", to_string(*ctx.from));}
    if (ctx.to)     {field("to", to_string(*ctx.to));}

    if (!first) {os << "\n";}
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

    if (ctx.z_threshold) {
        os << ",\n  \"anomalies\": {\n";
        os << "    \"method\": \"zscore\",\n";
        os << "    \"threshold\": " << *ctx.z_threshold << ",\n";
        os << "    \"top_n\": " << ctx.top_n << ",\n";
        os << "    \"mode\": \"global\",\n";
        os << "    \"global\": ";
        if (global_anomalies) {
            write_anomaly_summary(os, *global_anomalies, 4);
            os << "\n";
        } else {
            os << "{ \"count\": 0, \"top\": [] }\n";
        }
        os << "  }\n";
        os << "}";
        return;
    }

    os << "\n}\n";
}

void write_json_report(std::ostream &os,
                       const ReportContext &ctx,
                       const std::map<std::string, Stats> &per_sensor,
                       const std::optional<std::map<std::string, AnomalySummary>>& per_sensor_anomalies)
{
    os << "{\n";

    os << "  \"mode\": \"per_sensor\",\n";

    os << "  \"import\": {\n";
    os << "    \"parsed_ok\": " << ctx.parsed_ok << ",\n";
    os << "    \"skipped\": " << ctx.skipped << "\n";
    os << "  },\n";


    os << "  \"filter\": {\n";
    bool first = true;

    auto field = [&](const char* k, const std::string& v) {
        if (!first) {os << ",\n";}
        first = false;
        os << "    \"" << k << "\": \"" << v << "\"";
    };

    if (ctx.from)   {field("from", to_string(*ctx.from));}
    if (ctx.to)     {field("to", to_string(*ctx.to));}

    if (!first) {os << "\n";}
    os << "  },\n";

    os << "  \"data\": {\n";
    os << "    \"total\": " << ctx.total << ",\n";
    os << "    \"filtered\": " << ctx.filtered << "\n";
    os << "  },\n";

    os << "  \"stats_by_sensor\": {\n";

    bool first_s = true;
    for (const auto& [name, st] : per_sensor) {
        if (!first_s) {os << ",\n";}
        first_s = false;

        os << "    \"" << name << "\": {\n";
        os << "      \"count\": " << st.count << ",\n";
        os << "      \"min\": " << st.min << ",\n";
        os << "      \"max\": " << st.max << ",\n";
        os << "      \"mean\": " << st.mean << ",\n";
        os << "      \"stddev\": " << st.stddev << "\n";
        os << "    }";
    }

    os << "\n  }";

    if (ctx.z_threshold) {
        os << ",\n  \"anomalies\": {\n";
        os << "    \"method\": \"zscore\",\n";
        os << "    \"threshold\": " << *ctx.z_threshold << ",\n";
        os << "    \"top_n\": " << ctx.top_n << ",\n";
        os << "    \"mode\": \"per_sensor\",\n";
        os << "    \"per_sensor\": {\n";

        if (per_sensor_anomalies) {
            std::size_t i = 0;
            for (const auto& [sensor, sum] : *per_sensor_anomalies) {
                os << "      \"" << sensor << "\": ";
                write_anomaly_summary(os, sum, 6);
                os << (++i < per_sensor_anomalies->size() ? ",\n" : "\n");
            }
        }

        os << "    }\n";
        os << "  }\n";
        os << "}\n";
        return;
    }

    os << "\n}\n";
}

} // namespace pdt
