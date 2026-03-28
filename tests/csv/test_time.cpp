#include "pdt/csv/time.h"

#include <cassert>

int main() {
    using namespace pdt;

    auto ok = parse_iso8601("2026-02-18T12:34:56");
    assert(ok.has_value());

    auto bad1 = parse_iso8601("2026-02-18 12:34:56");   // missing 'T'
    assert(!bad1.has_value());

    auto bad2 = parse_iso8601("2026-13-18T12:34:56");   // invalid month
    assert(!bad1.has_value());

    auto bad3 = parse_iso8601("2026-02-30T12:34:56");   // invalid day
    assert(!bad1.has_value());

    return 0;
}
