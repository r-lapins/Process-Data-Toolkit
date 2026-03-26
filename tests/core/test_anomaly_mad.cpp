#include "pdt/core/anomaly.h"
#include "pdt/core/dataset.h"
#include "pdt/core/time.h"

#include <cassert>
#include <vector>

namespace {

pdt::Sample make_sample(std::string_view ts, std::string sensor, double value) {
    auto parsed = pdt::parse_iso8601(ts);
    assert(parsed.has_value());

    return pdt::Sample{
        .timestamp = *parsed,
        .sensor = std::move(sensor),
        .value = value
    };
}

} // namespace

int main() {
    using namespace pdt;
    using enum AnomalyMethod;

    {
        DataSet ds{std::vector<Sample>{
            make_sample("2026-02-18T10:00:00", "S1", 10.0),
            make_sample("2026-02-18T10:01:00", "S1", 11.0),
            make_sample("2026-02-18T10:02:00", "S1", 10.5),
            make_sample("2026-02-18T10:03:00", "S1", 9.5),
            make_sample("2026-02-18T10:04:00", "S1", 10.2),
            make_sample("2026-02-18T10:05:00", "S1", 100.0)
        }};

        const auto summary = detect_anomalies_global(ds, MAD, 3.0, 5);

        assert(summary.all.size() == 1);
        assert(summary.top.size() == 1);
        assert(summary.top[0].sensor == "S1");
        assert(summary.top[0].value == 100.0);
        assert(summary.top[0].score > 0.0);

        const auto unified = detect_anomalies_global(ds, MAD, 3.0, 5);
        assert(unified.all.size() == 1);
        assert(unified.top.size() == 1);
        assert(unified.top[0].value == 100.0);
    }

    {
        DataSet ds{std::vector<Sample>{
            make_sample("2026-02-18T10:00:00", "S1", 10.0),
            make_sample("2026-02-18T10:01:00", "S1", 11.0),
            make_sample("2026-02-18T10:02:00", "S1", 10.0),
            make_sample("2026-02-18T10:03:00", "S1", 11.0),

            make_sample("2026-02-18T10:00:00", "S2", 20.0),
            make_sample("2026-02-18T10:01:00", "S2", 21.0),
            make_sample("2026-02-18T10:02:00", "S2", 20.0),
            make_sample("2026-02-18T10:03:00", "S2", 100.0)
        }};

        const auto per_sensor = detect_anomalies_per_sensor(ds, MAD, 3.0, 5);

        assert(per_sensor.size() == 2);
        assert(per_sensor.contains("S1"));
        assert(per_sensor.contains("S2"));

        assert(per_sensor.at("S1").all.size() == 0);
        assert(per_sensor.at("S1").top.empty());

        assert(per_sensor.at("S2").all.size() == 1);
        assert(per_sensor.at("S2").top.size() == 1);
        assert(per_sensor.at("S2").top[0].sensor == "S2");
        assert(per_sensor.at("S2").top[0].value == 100.0);

        const auto unified = detect_anomalies_per_sensor(ds, MAD, 3.0, 5);

        assert(unified.size() == 2);
        assert(unified.contains("S1"));
        assert(unified.contains("S2"));
        assert(unified.at("S1").all.size() == 0);
        assert(unified.at("S2").all.size() == 1);
        assert(unified.at("S2").top.size() == 1);
        assert(unified.at("S2").top[0].value == 100.0);
    }

    {
        DataSet ds{std::vector<Sample>{
            make_sample("2026-02-18T10:00:00", "S1", 5.0),
            make_sample("2026-02-18T10:01:00", "S1", 5.0),
            make_sample("2026-02-18T10:02:00", "S1", 5.0),
            make_sample("2026-02-18T10:03:00", "S1", 5.0),
            make_sample("2026-02-18T10:04:00", "S1", 100.0)
        }};

        const auto summary = detect_anomalies_global(ds, MAD, 3.0, 5);

        assert(summary.all.size() == 0);
        assert(summary.top.empty());
    }

    {
        DataSet ds{std::vector<Sample>{
            make_sample("2026-02-18T10:00:00", "S1", 10.0),
            make_sample("2026-02-18T10:01:00", "S1", 10.0),
            make_sample("2026-02-18T10:02:00", "S1", 11.0),
            make_sample("2026-02-18T10:03:00", "S1", 12.0),
            make_sample("2026-02-18T10:04:00", "S1", 100.0),
            make_sample("2026-02-18T10:05:00", "S1", 120.0)
        }};

        const auto summary = detect_anomalies_global(ds, MAD, 3.0, 1);

        assert(summary.all.size() == 2);
        assert(summary.top.size() == 1);
    }

    {
        DataSet ds{std::vector<Sample>{
            make_sample("2026-02-18T10:00:00", "S1", 10.0),
            make_sample("2026-02-18T10:01:00", "S1", 100.0)
        }};

        const auto summary = detect_anomalies_global(ds, MAD, 3.0, 5);

        assert(summary.all.size() == 0);
        assert(summary.top.empty());
    }

    {
        DataSet ds{std::vector<Sample>{
            make_sample("2026-02-18T10:00:00", "S1", 10.0),
            make_sample("2026-02-18T10:01:00", "S1", 11.0),
            make_sample("2026-02-18T10:02:00", "S1", 10.5),
            make_sample("2026-02-18T10:03:00", "S1", 9.5),
            make_sample("2026-02-18T10:04:00", "S1", 10.2),
            make_sample("2026-02-18T10:05:00", "S1", 100.0)
        }};

        const auto summary_invalid_threshold = detect_anomalies_global(ds, MAD, 0.0, 5);
        assert(summary_invalid_threshold.all.size() == 0);
        assert(summary_invalid_threshold.top.empty());

        const auto summary_zero_top_n = detect_anomalies_global(ds, MAD, 3.0, 0);
        assert(summary_zero_top_n.all.size() == 0);
        assert(summary_zero_top_n.top.empty());
    }

    return 0;
}