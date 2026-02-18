#include "pdt/stats.h"
#include <cassert>
#include <vector>

int main() {
    using namespace pdt;

    std::vector<Sample> samples = {
        {{}, "S1", 1.0},
        {{}, "S1", 2.0},
        {{}, "S1", 3.0}
    };

    Stats s = compute_stats(samples);

    assert(s.count == 3);
    assert(s.mean == 2.0);
    assert(s.min == 1.0);
    assert(s.max == 3.0);

    return 0;
}
