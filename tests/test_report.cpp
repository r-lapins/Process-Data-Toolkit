#include "pdt/report.h"
#include <cassert>
#include <sstream>

int main() {
    using namespace pdt;

    ReportContext ctx{};
    ctx.parsed_ok = 3;
    ctx.skipped = 1;
    ctx.total = 3;
    ctx.filtered = 2;
    ctx.sensor = std::string{"S1"};

    Stats st{};
    st.count = 2;
    st.min = 1.0;
    st.max = 3.0;
    st.mean = 2.0;
    st.stddev = 1.0;

    std::stringstream ss;
    write_json_report(ss, ctx, st);

    auto out = ss.str();

    assert(out.find("\"parsed_ok\": 3") != std::string::npos);
    assert(out.find("\"sensor\": \"S1\"") != std::string::npos);
    assert(out.find("\"mean\": 2") != std::string::npos);

    return 0;
}
