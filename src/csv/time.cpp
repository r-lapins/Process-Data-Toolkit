#include "pdt/csv/time.h"

#include <iomanip>
#include <sstream>

namespace pdt {
namespace {

bool is_digit(char c) { return c >= '0' && c <= '9'; }

bool parse_2(std::string_view s, std::size_t pos, int& out)
{
    if (pos + 2 > s.size()) { return false; }

    const char a = s[pos];
    const char b = s[pos + 1];

    if (!is_digit(a) || !is_digit(b)) { return false; }

    out = ((a - '0') * 10) + (b - '0');
    return true;
}

bool parse_4(std::string_view s, std::size_t pos, int& out)
{
    if (pos + 4 > s.size()) { return false; }

    for (std::size_t i = 0; i < 4; ++i) {
        if (!is_digit(s[pos + i])) { return false; }
    }

    out = (s[pos] - '0') * 1000
          + (s[pos + 1] - '0') * 100
          + (s[pos + 2] - '0') * 10
          + (s[pos + 3] - '0');

    return true;
}

std::string format_timestamp(std::chrono::sys_seconds ts, std::string_view separator)
{
    using namespace std::chrono;

    const auto dp = floor<days>(ts);
    const year_month_day ymd{dp};
    const auto time = hh_mm_ss{ts - dp};

    std::ostringstream oss;
    oss << std::setw(4) << std::setfill('0') << int(ymd.year()) << "-"
        << std::setw(2) << unsigned(ymd.month()) << "-"
        << std::setw(2) << unsigned(ymd.day()) << separator
        << std::setw(2) << time.hours().count() << ":"
        << std::setw(2) << time.minutes().count() << ":"
        << std::setw(2) << time.seconds().count();

    return oss.str();
}

} // namespace

std::optional<std::chrono::sys_seconds> parse_iso8601(std::string_view s)
{
    if (s.size() != 19) { return std::nullopt; }
    if (s[4] != '-' || s[7] != '-' || s[10] != 'T' || s[13] != ':' || s[16] != ':') {
        return std::nullopt;
    }

    int y{};
    int mo{};
    int d{};
    int h{};
    int mi{};
    int sec{};
    if (!parse_4(s, 0, y))      { return std::nullopt; }
    if (!parse_2(s, 5, mo))     { return std::nullopt; }
    if (!parse_2(s, 8, d))      { return std::nullopt; }
    if (!parse_2(s, 11, h))     { return std::nullopt; }
    if (!parse_2(s, 14, mi))    { return std::nullopt; }
    if (!parse_2(s, 17, sec))   { return std::nullopt; }

    using namespace std::chrono;

    // Range checks
    if (mo < 1 || mo > 12)      { return std::nullopt; }
    if (h < 0 || h > 23)        { return std::nullopt; }
    if (mi < 0 || mi > 59)      { return std::nullopt; }
    if (sec < 0 || sec > 59)    { return std::nullopt; }

    year_month_day ymd{year{y}, month{static_cast<unsigned>(mo)}, day{static_cast<unsigned>(d)}};
    if (!ymd.ok()) { return std::nullopt; }

    const sys_days day_point{ymd};
    const sys_seconds ts = time_point_cast<seconds>(day_point) + hours{h} + minutes{mi} + seconds{sec};

    return ts;
}

std::string format_iso8601(std::chrono::sys_seconds ts)
{
    return format_timestamp(ts, "T");
}

std::string format_date_time(std::chrono::sys_seconds ts)
{
    return format_timestamp(ts, " ");
}

} // namespace pdt
