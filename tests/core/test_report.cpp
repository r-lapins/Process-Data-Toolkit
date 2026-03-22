#include "pdt/core/report.h"

#include <cassert>
#include <map>
#include <sstream>
#include <string>

int main() {
    using namespace pdt;

    ReportContext ctx{};
    ctx.parsed_ok = 3;
    ctx.skipped = 1;
    ctx.total = 3;
    ctx.filtered = 2;
    ctx.sensor = std::string{"S1"};
    ctx.anomaly_threshold = 3.0;
    ctx.anomaly_method = AnomalyMethod::ZScore;
    ctx.top_n = 10;

    // 1) global stats
    {
        Stats st{};
        st.mean = 2;
        st.min = 1;
        st.max = 3;
        st.stddev = 1;
        st.count = 2;

        std::stringstream ss;
        write_json_report(ss, ctx, st, std::nullopt);

        const auto s = ss.str();
        assert(s.find("\"stats\"") != std::string::npos);
        assert(s.find("\"mean\": 2") != std::string::npos);
    }

    // 2) per-sensor stats
    {
        std::map<std::string, Stats> per;
        per["S1"] = Stats{2, 1, 3, 1, 2};

        std::stringstream ss;
        write_json_report(ss, ctx, per, std::nullopt);

        const auto s = ss.str();
        assert(s.find("\"stats_by_sensor\"") != std::string::npos);
        assert(s.find("\"S1\"") != std::string::npos);
    }

    // 3) anomalies (global)
    {
        Stats st{};
        st.mean = 2;
        st.min = 1;
        st.max = 3;
        st.stddev = 1;
        st.count = 2;

        AnomalySummary an{};
        an.count = 1;
        an.top.push_back(Anomaly{std::chrono::sys_seconds{}, "S1", 123.0, 4.2});

        std::stringstream ss;
        write_json_report(ss, ctx, st, an);

        const auto s = ss.str();
        assert(s.find("\"anomalies\"") != std::string::npos);
        assert(s.find("\"method\": \"zscore\"") != std::string::npos);
        assert(s.find("\"score\":4.2") != std::string::npos);
    }

    // 4) anomalies (per-sensor)
    {
        ReportContext per_ctx{};
        per_ctx.parsed_ok = 5;
        per_ctx.skipped = 0;
        per_ctx.total = 5;
        per_ctx.filtered = 5;
        per_ctx.anomaly_threshold = 3.0;
        per_ctx.anomaly_method = AnomalyMethod::ZScore;
        per_ctx.top_n = 10;

        std::map<std::string, Stats> per_stats;
        per_stats["S1"] = Stats{2.0, 1.0, 3.0, 1.0, 2};
        per_stats["S2"] = Stats{15.0, 10.0, 20.0, 5.0, 3};

        std::map<std::string, AnomalySummary> per_anoms;

        AnomalySummary s1_sum{};
        s1_sum.count = 1;
        s1_sum.top.push_back(
            Anomaly{std::chrono::sys_seconds{}, "S1", 123.0, 4.5});

        AnomalySummary s2_sum{};
        s2_sum.count = 2;
        s2_sum.top.push_back(
            Anomaly{std::chrono::sys_seconds{}, "S2", 999.0, -5.0});

        per_anoms["S1"] = s1_sum;
        per_anoms["S2"] = s2_sum;

        std::stringstream ss;
        write_json_report(ss, per_ctx, per_stats, per_anoms);

        const auto s = ss.str();

        assert(s.find("\"stats_by_sensor\"") != std::string::npos);
        assert(s.find("\"anomalies\"") != std::string::npos);
        assert(s.find("\"mode\": \"per_sensor\"") != std::string::npos);
        assert(s.find("\"method\": \"zscore\"") != std::string::npos);
        assert(s.find("\"S1\"") != std::string::npos);
        assert(s.find("\"S2\"") != std::string::npos);
        assert(s.find("\"score\":4.5") != std::string::npos);
        assert(s.find("\"score\":-5") != std::string::npos);
    }

    return 0;
}