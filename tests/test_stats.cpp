#include "pdt/types.h"
#include "pdt/dataset.h"
#include <cassert>
#include <vector>

int main() {
    using namespace pdt;

    DataSet ds{std::vector<Sample>{
        {{}, "S1", 1.0},
        {{}, "S1", 2.0},
        {{}, "S1", 3.0}
    }};

    Stats s = ds.stats();

    assert(s.count == 3);
    assert(s.mean == 2.0);
    assert(std::abs(s.mean - 2.0) < 1e-12); // More secure version
    assert(s.min == 1.0);
    assert(s.max == 3.0);

    return 0;
}
