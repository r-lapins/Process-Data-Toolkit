#include "pdt/filter.h"
#include "pdt/time.h"
#include <cassert>
#include <vector>

int main() {
    using namespace pdt;

    auto t1 = *parse_iso8601("2026-02-18T10:00:00");
    auto t2 = *parse_iso8601("2026-02-18T11:00:00");
    auto t3 = *parse_iso8601("2026-02-18T12:00:00");

    std::vector<Sample> samples = {
        {t1, "S1", 1.0},
        {t2, "S2", 2.0},
        {t3, "S1", 3.0},
        };

    FilterOptions opt{};
    opt.sensor = std::string{"S1"};
    opt.from = t2;   // inclusive
    auto out = filter_samples(samples, opt);

    assert(out.size() == 1);
    assert(out[0].sensor == "S1");
    assert(out[0].timestamp == t3);

    return 0;
}
