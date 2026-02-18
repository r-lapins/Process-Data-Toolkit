#include "pdt/csv_reader.h"
#include <cassert>
#include <sstream>

int main() {
    using namespace pdt;

    std::stringstream ss;
    ss << "timestamp,sensor,value\n";
    ss << "t1,S1,1.0\n";
    ss << "t2,S1,2.0\n";
    ss << "bad_line\n";
    ss << "t3,S1,abc\n"; // bad value

    ImportResult r = read_csv(ss);

    // aborts the program when false
    assert(r.parsed_ok == 2);
    assert(r.skipped == 2);
    assert(r.samples.size() == 2);

    return 0;
}
