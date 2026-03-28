#pragma once

#include <chrono>
#include <optional>
#include <string_view>

namespace pdt {

// Parses "YYYY-MM-DDTHH:MM:SS" into sys_seconds.
// Returns nullopt on invalid format/ranges.

std::optional<std::chrono::sys_seconds> parse_iso8601(std::string_view s);

} // namespace pdt
