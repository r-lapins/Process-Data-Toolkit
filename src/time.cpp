#include "pdt/time.h"
#include <cctype>

namespace pdt {
namespace {

constexpr bool is_digit(char c) { return c >= '0' && c <= '9'; }

bool parse_2(std::string_view s, std::size_t pos, int& out) {
    if (pos + 2 > s.size()) return false;

    char a = s[pos], b = s [pos + 1];
    if (!is_digit(a) || !is_digit(b)) return false;

    out = (a - '0') * 10 + (b - '0');
    return true;
}

bool parse_4(std::string_view s, std::size_t pos, int& out) {
    if (pos + 4 > s.size()) return false;

    for (std::size_t i = 0; i < 4; ++i)
        if (!is_digit(s[pos + i])) return false;

    out = (s[pos] - '0') * 1000 + (s[pos + 1] - '0') * 100 + (s[pos + 2] - '0') * 10 + (s[pos + 3] - '0');
    return true;
}

} // namespace

std::optional<std::chrono::sys_seconds> parse_iso8601(std::string_view s) {
    // Expected: YYYY-MM-DDTHH:MM:SS (length 19)
    if (s.size() != 19) return std::nullopt;
    if (s[4] != '-' || s[7] != '-' || s[10] != 'T' || s[13] != ':' || s[16] != ':')
        return std::nullopt;

    int y{}, mo{}, d{}, h{}, mi{}, sec{};
    if (!parse_4(s, 0, y)) return std::nullopt;
    if (!parse_2(s, 5, mo)) return std::nullopt;
    if (!parse_2(s, 8, d)) return std::nullopt;
    if (!parse_2(s, 11, h)) return std::nullopt;
    if (!parse_2(s, 14, mi)) return std::nullopt;
    if (!parse_2(s, 17, sec)) return std::nullopt;

    using namespace std::chrono;

    // Range checks
    if (mo < 1 || mo > 12) return std::nullopt;
    if (h < 0 || mo > 23) return std::nullopt;
    if (mi < 0 || mi > 59) return std::nullopt;
    if (sec < 0 || sec > 59) return std::nullopt;

    year_month_day ymd{year{y}, month{static_cast<unsigned>(mo)}, day{static_cast<unsigned>(d)}};
    if (!ymd.ok()) return std::nullopt;

    sys_days day_point{ymd};
    sys_seconds ts = time_point_cast<seconds>(day_point) + hours{h} + minutes{mi} + seconds{sec};
    return ts;
}

} // namespace pdt
