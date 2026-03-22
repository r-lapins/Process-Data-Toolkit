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

    DataSet ds{std::vector<Sample>{
        make_sample("2026-02-18T10:00:00", "S1", 10.0),
        make_sample("2026-02-18T10:01:00", "S1", 11.0),
        make_sample("2026-02-18T10:02:00", "S1", 10.5),
        make_sample("2026-02-18T10:03:00", "S1", 9.5),
        make_sample("2026-02-18T10:04:00", "S1", 10.2),
        make_sample("2026-02-18T10:05:00", "S1", 100.0)
    }};

    const auto summary = detect_anomalies_global(ds, IQR, 1.5, 5);

    assert(summary.count == 1);
    assert(summary.top.size() == 1);
    assert(summary.top[0].sensor == "S1");
    assert(summary.top[0].value == 100.0);
    assert(summary.top[0].score > 0.0);

    const auto unified = detect_anomalies_global(ds, IQR, 1.5, 5);
    assert(unified.count == 1);
    assert(unified.top.size() == 1);
    assert(unified.top[0].value == 100.0);

    return 0;
}
