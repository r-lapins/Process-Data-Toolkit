#include "pdt/core/anomaly.h"
#include "pdt/core/time.h"

#include <cassert>
#include <vector>

namespace {
using namespace pdt;

Sample mk(const char* ts, const char* sensor, double v) {
    auto t = parse_iso8601(ts);
    assert(t);
    return Sample{*t, std::string{sensor}, v};
}
} // namespace

int main() {
    using namespace pdt;
    using enum AnomalyMethod;

    DataSet ds{std::vector<Sample>{
        mk("2026-02-18T10:00:00", "S1", 10.0),
        mk("2026-02-18T10:01:00", "S1", 10.1),
        mk("2026-02-18T10:02:00", "S1", 9.9),
        mk("2026-02-18T10:03:00", "S1", 100.0) // outlier
    }};

    auto sum = detect_anomalies_global(ds, ZScore, 1.5, 10);
    assert(sum.count >= 1);
    assert(!sum.top.empty());
    assert(sum.top[0].value == 100.0);

    auto per = detect_anomalies_per_sensor(ds, ZScore, 1.5, 10);
    assert(per.size() == 1);
    assert(per.at("S1").count >= 1);
    assert(per.at("S1").top[0].value == 100.0);

    return 0;
}
