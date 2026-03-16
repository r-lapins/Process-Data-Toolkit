#include "pdt/core/dataset.h"
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

    DataSet ds{std::vector<Sample>{
        mk("2026-02-18T10:00:00", "S1", 1.0),
        mk("2026-02-18T11:00:00", "S2", 10.0),
        mk("2026-02-18T12:00:00", "S1", 3.0),
        mk("2026-02-18T13:00:00", "S2", 20.0),
        mk("2026-02-18T14:00:00", "S3", -5.0),
    }};

    auto m = ds.stats_by_sensor();
    assert(m.size() == 3);

    {
        const auto& s1 = m.at("S1");
        assert(s1.count == 2);
        assert(s1.min == 1.0);
        assert(s1.max == 3.0);
        assert(s1.mean == 2.0);
    }
    {
        const auto& s2 = m.at("S2");
        assert(s2.count == 2);
        assert(s2.min == 10.0);
        assert(s2.max == 20.0);
        assert(s2.mean == 15.0);
    }
    {
        const auto& s3 = m.at("S3");
        assert(s3.count == 1);
        assert(s3.min == -5.0);
        assert(s3.max == -5.0);
        assert(s3.mean == -5.0);
    }

    return 0;
}
