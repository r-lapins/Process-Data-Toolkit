#include "pdt/report.h"

namespace {

std::string to_string(std::chrono::sys_seconds ts) {
    using namespace std::chrono;
    auto dp = floor<days>(ts);
    year_month_day ymd{dp};

    auto time = hh_mm_ss{ts - dp};

    std::ostringstream oss;
    oss << std::setw(4) << std::setfill('0') << int(ymd.year()) << "-"
        << std::setw(2) << unsigned(ymd.month()) << "-"
        << std::setw(2) << unsigned(ymd.day()) << "T"
        << std::setw(2) << time.hours().count() << ":"
        << std::setw(2) << time.minutes().count() << ":"
        << std::setw(2) << time.seconds().count();
    return oss.str();
}

} // namespace

namespace pdt {

void write_json_report(std::ostream& os,
                       const pdt::ReportContext& ctx,
                       const pdt::Stats& stats)
{
    os << "{\n";

    os << "  \"import\": {\n";
    os << "    \"parsed_ok\": " << ctx.parsed_ok << ",\n";
    os << "    \"skipped\": " << ctx.skipped << "\n";
    os << "  },\n";

    os << "  \"filter\": {\n";
    if (ctx.sensor)
        os << "    \"sensor\": \"" << *ctx.sensor << "\",\n";
    if (ctx.from)
        os << "    \"from\": \"" << to_string(*ctx.from) << "\",\n";
    if (ctx.to)
        os << "    \"to\": \"" << to_string(*ctx.to) << "\",\n";
    os << "  },\n";

    os << "  \"data\": {\n";
    os << "    \"total\": " << ctx.total << ",\n";
    os << "    \"filtered\": " << ctx.filtered << "\n";
    os << "  },\n";

    os << "  \"stats\": {\n";
    os << "    \"count\": " << stats.count << ",\n";
    os << "    \"min\": " << stats.min << ",\n";
    os << "    \"max\": " << stats.max << ",\n";
    os << "    \"mean\": " << stats.mean << ",\n";
    os << "    \"stddev\": " << stats.stddev << "\n";
    os << "  }\n";

    os << "}\n";
}

} // namespace pdt
