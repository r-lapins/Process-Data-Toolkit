#include "pdt/core/dataset.h"
#include "pdt/core/time.h"

#include <cassert>
#include <vector>

using namespace pdt;

namespace {

Sample mk(std::string_view ts, std::string sensor, double v) {
    auto t = parse_iso8601(ts);
    assert(t.has_value());
    return Sample{*t, std::move(sensor), v};
}
} // namespace

int main() {
    DataSet ds{std::vector<Sample>{
            mk("2026-02-18T10:00:00", "S1", 1.0),
            mk("2026-02-18T11:00:00", "S2", 2.0),
            mk("2026-02-18T12:00:00", "S1", 3.0),
    }};

    assert(ds.size() == 3);
    assert(!ds.empty());

    FilterOptions opt{};
    opt.sensor = std::string{"S1"};

    auto s1 = ds.filter(opt);
    assert(s1.size() == 2);

    auto st = s1.stats();
    assert(st.count == 2);
    assert(st.min == 1.0);
    assert(st.max == 3.0);
    assert(st.mean == 2.0);

    return 0;
}
