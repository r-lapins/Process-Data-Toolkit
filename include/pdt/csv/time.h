#pragma once

#include <chrono>
#include <optional>
#include <string>
#include <string_view>

namespace pdt {

// Parses "YYYY-MM-DDTHH:MM:SS" into sys_seconds.
// Returns nullopt on invalid format/ranges.
std::optional<std::chrono::sys_seconds> parse_iso8601(std::string_view s);

// Formats timestamp as "YYYY-MM-DDTHH:MM:SS".
std::string format_iso8601(std::chrono::sys_seconds ts);

// Formats timestamp as "YYYY-MM-DD  HH:MM:SS".
std::string format_date_time(std::chrono::sys_seconds ts);

} // namespace pdt