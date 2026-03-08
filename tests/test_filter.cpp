#include "pdt/dataset.h"
#include "pdt/types.h"
#include "pdt/time.h"
#include <cassert>
#include <vector>

// using namespace pdt;

namespace {

std::chrono::sys_seconds must_parse(std::string_view ts) {
    auto parsed = pdt::parse_iso8601(ts);
    assert(parsed.has_value());
    return *parsed;
}

pdt::Sample mk(std::string_view ts, std::string sensor, double v) {
    return pdt::Sample{
        .timestamp = must_parse(ts),
        .sensor = std::move(sensor),
        .value = v
    };
}

pdt::FilterOptions opt_sensor(std::string s) {
    pdt::FilterOptions o{};
    o.sensor = std::move(s);
    return o;
}

pdt::FilterOptions opt_from(std::string_view from) {
    pdt::FilterOptions o{};
    auto t = pdt::parse_iso8601(from);
    assert(t.has_value());
    o.from = *t;
    return o;
}

pdt::FilterOptions opt_to(std::string_view to) {
    pdt::FilterOptions o{};    
    auto t = pdt::parse_iso8601(to);
    assert(t.has_value());
    o.to = *t;
    return o;
}

void assert_same_order_by_value(const pdt::DataSet& out, const std::vector<double>& values) {
    assert(out.size() == values.size());
    std::size_t i = 0;
    for (const auto& s : out) {
        assert(s.value == values[i]);
        ++i;
    }
}

} // namespace


int main() {
    // Note: values are treated as sample ‘IDs’ in tests.
    // This makes it easy to check the order and completeness of results.
    pdt::DataSet ds{std::vector<pdt::Sample>{
        mk("2026-02-18T10:00:00", "S1", 1.0),
        mk("2026-02-18T10:30:00", "S2", 2.0),
        mk("2026-02-18T11:00:00", "S1", 3.0),
        mk("2026-02-18T11:30:00", "S2", 4.0),
        mk("2026-02-18T12:00:00", "S1", 5.0),
        mk("2026-02-18T12:30:00", "S3", 6.0),
    }};

           // 1) No options => everything, without changing the order
    {
        pdt::FilterOptions o{};
        auto out = ds.filter(o);
        assert(out.size() == ds.size());
        assert_same_order_by_value(out, {1,2,3,4,5,6});
    }

           // 2) Sensor only (matches)
    {
        auto out = ds.filter(opt_sensor("S1"));
        assert_same_order_by_value(out, {1,3,5});
    }

           // 3) Sensor only (no matches)
    {
        auto out = ds.filter(opt_sensor("NOPE"));
        assert(out.empty());
    }

           // 4) 'from' only (inclusive)
    {
        auto out = ds.filter(opt_from("2026-02-18T11:00:00"));
        // od 11:00:00 włącznie => 3,4,5,6
        assert_same_order_by_value(out, {3,4,5,6});
    }

           // 5) 'to' only (inclusive)
    {
        auto out = ds.filter(opt_to("2026-02-18T11:00:00"));
        // do 11:00:00 włącznie => 1,2,3
        assert_same_order_by_value(out, {1,2,3});
    }

           // 6) from + to (time window, inclusive)
    {
        pdt::FilterOptions o{};
        o.from = must_parse("2026-02-18T10:30:00");
        o.to   = must_parse("2026-02-18T12:00:00");
        auto out = ds.filter(o);
        // 10:30..12:00 => 2,3,4,5
        assert_same_order_by_value(out, {2,3,4,5});
    }

           // 7) sensor + from + to (combined)
    {
        pdt::FilterOptions o{};
        o.sensor = std::string{"S2"};
        o.from = must_parse("2026-02-18T10:00:00");
        o.to = must_parse("2026-02-18T12:00:00");

        auto out = ds.filter(o);
        // S2 w oknie => 2,4
        assert_same_order_by_value(out, {2,4});
    }

           // 8) Bounds: exactly one point in time (from==to)
    {
        pdt::FilterOptions o{};
        o.from = must_parse("2026-02-18T11:30:00");
        o.to   = must_parse("2026-02-18T11:30:00");
        auto out = ds.filter(o);
        // dokładnie 11:30:00 => tylko value 4.0
        assert_same_order_by_value(out, {4});
    }

           // 9) Edge-case: from > to => sensible behaviour: empty
    {
        pdt::FilterOptions o{};
        o.from = must_parse("2026-02-18T12:00:00");
        o.to   = must_parse("2026-02-18T10:00:00");
        auto out = ds.filter(o);
        assert(out.empty());
    }

           // 10) Verify that filtering does not mutate the input
    {
        auto before = ds.size();
        auto out = ds.filter(opt_sensor("S3"));
        (void)out;  // to avoid the warning: unused variable 'out'
        assert(ds.size() == before);
    }

    return 0;
}
