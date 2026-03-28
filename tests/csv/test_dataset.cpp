#include "pdt/csv/dataset.h"
#include "pdt/csv/time.h"

#include <cassert>
#include <cmath>
#include <string_view>
#include <vector>

namespace {

using namespace pdt;

constexpr double eps = 1e-9;

Sample make_sample(std::string_view ts, std::string sensor, double value) {
    const auto parsed = parse_iso8601(ts);
    assert(parsed.has_value());
    return Sample{*parsed, std::move(sensor), value};
}

bool almost_equal(double a, double b, double tol = eps) { return std::abs(a - b) < tol; }

} // namespace

int main() {
    using namespace pdt;

    // 1. Empty dataset
    {
        DataSet ds{};

        const auto st = ds.stats();

        assert(ds.empty());
        assert(ds.size() == 0);

        assert(st.count == 0);
        assert(almost_equal(st.mean, 0.0));
        assert(almost_equal(st.min, 0.0));
        assert(almost_equal(st.max, 0.0));
        assert(almost_equal(st.stddev, 0.0));
        assert(almost_equal(st.median, 0.0));
        assert(almost_equal(st.q1, 0.0));
        assert(almost_equal(st.q3, 0.0));
    }

    // 2. Single element
    {
        DataSet ds{std::vector<Sample>{make_sample("2026-02-18T10:00:00", "S1", 5.0)}};

        const auto st = ds.stats();

        assert(st.count == 1);
        assert(almost_equal(st.mean, 5.0));
        assert(almost_equal(st.min, 5.0));
        assert(almost_equal(st.max, 5.0));
        assert(almost_equal(st.stddev, 0.0));
        assert(almost_equal(st.median, 5.0));
        assert(almost_equal(st.q1, 5.0));
        assert(almost_equal(st.q3, 5.0));
    }

    // 3. Odd number of values: [1, 2, 3, 4, 5]
    {
        DataSet ds{std::vector<Sample>{
            make_sample("2026-02-18T10:00:00", "S1", 1.0),
            make_sample("2026-02-18T10:01:00", "S1", 2.0),
            make_sample("2026-02-18T10:02:00", "S1", 3.0),
            make_sample("2026-02-18T10:03:00", "S1", 4.0),
            make_sample("2026-02-18T10:04:00", "S1", 5.0),
        }};

        const auto st = ds.stats();

        assert(st.count == 5);
        assert(almost_equal(st.min, 1.0));
        assert(almost_equal(st.max, 5.0));
        assert(almost_equal(st.mean, 3.0));
        assert(almost_equal(st.median, 3.0));
        assert(almost_equal(st.q1, 2.0));
        assert(almost_equal(st.q3, 4.0));
    }

    // 4. Even number of values: [1, 2, 3, 4]
    // Linear interpolation quantiles:
    // median = 2.5, q1 = 1.75, q3 = 3.25
    {
        DataSet ds{std::vector<Sample>{
            make_sample("2026-02-18T10:00:00", "S1", 1.0),
            make_sample("2026-02-18T10:01:00", "S1", 2.0),
            make_sample("2026-02-18T10:02:00", "S1", 3.0),
            make_sample("2026-02-18T10:03:00", "S1", 4.0),
        }};

        const auto st = ds.stats();

        assert(st.count == 4);
        assert(almost_equal(st.mean, 2.5));
        assert(almost_equal(st.median, 2.5));
        assert(almost_equal(st.q1, 1.75));
        assert(almost_equal(st.q3, 3.25));
    }

    // 5. stats_by_sensor()
    {
        DataSet ds{std::vector<Sample>{
            make_sample("2026-02-18T10:00:00", "S1", 1.0),
            make_sample("2026-02-18T10:01:00", "S1", 3.0),
            make_sample("2026-02-18T10:02:00", "S2", 10.0),
            make_sample("2026-02-18T10:03:00", "S2", 20.0),
            make_sample("2026-02-18T10:04:00", "S2", 30.0),
        }};

        const auto per = ds.stats_by_sensor();

        assert(per.size() == 2);

        const auto &s1 = per.at("S1");
        assert(s1.count == 2);
        assert(almost_equal(s1.mean, 2.0));
        assert(almost_equal(s1.median, 2.0));
        assert(almost_equal(s1.q1, 1.5));
        assert(almost_equal(s1.q3, 2.5));

        const auto &s2 = per.at("S2");
        assert(s2.count == 3);
        assert(almost_equal(s2.mean, 20.0));
        assert(almost_equal(s2.median, 20.0));
        assert(almost_equal(s2.q1, 15.0));
        assert(almost_equal(s2.q3, 25.0));
    }

    return 0;
}