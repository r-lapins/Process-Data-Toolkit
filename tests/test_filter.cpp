#include "pdt/filter.h"
#include "pdt/time.h"
#include <cassert>
#include <vector>

using namespace pdt;

namespace {

Sample mk(std::string_view ts, std::string sensor, double v) {
    auto t = parse_iso8601(ts);
    assert(t.has_value());
    return Sample{*t, std::move(sensor), v};
}

FilterOptions opt_sensor(std::string s) {
    FilterOptions o{};
    o.sensor = std::move(s);
    return o;
}

FilterOptions opt_from(std::string_view from) {
    FilterOptions o{};
    o.from = *parse_iso8601(from);
    return o;
}

FilterOptions opt_to(std::string_view to) {
    FilterOptions o{};
    o.to = *parse_iso8601(to);
    return o;
}

FilterOptions opt_sensor_from_to(std::string s, std::string_view from, std::string_view to) {
    FilterOptions o{};
    o.sensor = std::move(s);
    o.from = *parse_iso8601(from);
    o.to = *parse_iso8601(to);
    return o;
}

void assert_same_order_by_value(const std::vector<Sample>& out, const std::vector<double>& values) {
    assert(out.size() == values.size());
    for (std::size_t i = 0; i < values.size(); ++i) {
        assert(out[i].value == values[i]);
    }
}

} // namespace


int main() {
    // Note: values are treated as sample ‘IDs’ in tests.
    // This makes it easy to check the order and completeness of results.
    const std::vector<Sample> samples = {
        mk("2026-02-18T10:00:00", "S1", 1.0),
        mk("2026-02-18T10:30:00", "S2", 2.0),
        mk("2026-02-18T11:00:00", "S1", 3.0),
        mk("2026-02-18T11:30:00", "S2", 4.0),
        mk("2026-02-18T12:00:00", "S1", 5.0),
        mk("2026-02-18T12:30:00", "S3", 6.0),
    };

           // 1) No options => everything, without changing the order
    {
        FilterOptions o{};
        auto out = filter_samples(samples, o);
        assert(out.size() == samples.size());
        assert_same_order_by_value(out, {1,2,3,4,5,6});
    }

           // 2) Sensor only (matches)
    {
        auto out = filter_samples(samples, opt_sensor("S1"));
        assert_same_order_by_value(out, {1,3,5});
    }

           // 3) Sensor only (no matches)
    {
        auto out = filter_samples(samples, opt_sensor("NOPE"));
        assert(out.empty());
    }

           // 4) 'from' only (inclusive)
    {
        auto out = filter_samples(samples, opt_from("2026-02-18T11:00:00"));
        // od 11:00:00 włącznie => 3,4,5,6
        assert_same_order_by_value(out, {3,4,5,6});
    }

           // 5) 'to' only (inclusive)
    {
        auto out = filter_samples(samples, opt_to("2026-02-18T11:00:00"));
        // do 11:00:00 włącznie => 1,2,3
        assert_same_order_by_value(out, {1,2,3});
    }

           // 6) from + to (time window, inclusive)
    {
        FilterOptions o{};
        o.from = *parse_iso8601("2026-02-18T10:30:00");
        o.to   = *parse_iso8601("2026-02-18T12:00:00");
        auto out = filter_samples(samples, o);
        // 10:30..12:00 => 2,3,4,5
        assert_same_order_by_value(out, {2,3,4,5});
    }

           // 7) sensor + from + to (combined)
    {
        auto out = filter_samples(samples, opt_sensor_from_to("S2", "2026-02-18T10:00:00", "2026-02-18T12:00:00"));
        // S2 w oknie => 2,4
        assert_same_order_by_value(out, {2,4});
    }

           // 8) Bounds: exactly one point in time (from==to)
    {
        FilterOptions o{};
        o.from = *parse_iso8601("2026-02-18T11:30:00");
        o.to   = *parse_iso8601("2026-02-18T11:30:00");
        auto out = filter_samples(samples, o);
        // dokładnie 11:30:00 => tylko value 4.0
        assert_same_order_by_value(out, {4});
    }

           // 9) Edge-case: from > to => sensible behaviour: empty
    {
        FilterOptions o{};
        o.from = *parse_iso8601("2026-02-18T12:00:00");
        o.to   = *parse_iso8601("2026-02-18T10:00:00");
        auto out = filter_samples(samples, o);
        assert(out.empty());
    }

           // 10) Verify that filtering does not mutate the input
    {
        auto before = samples.size();
        auto out = filter_samples(samples, opt_sensor("S3"));
        (void)out;  // to avoid the warning: unused variable 'out'
        assert(samples.size() == before);
    }

    return 0;
}
