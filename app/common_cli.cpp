#include "common_cli.h"

#include <string>

namespace cli_common {

bool is_option(std::string_view text) {
    return !text.empty() && text.front() == '-';
}

bool parse_size_t(std::string_view text, std::size_t& out) {
    if (!text.empty() && text.front() == '-') {
        return false;
    }

    try {
        std::size_t pos = 0;
        const auto value = std::stoull(std::string{text}, &pos);
        if (pos != text.size()) {
            return false;
        }
        out = static_cast<std::size_t>(value);
        return true;
    } catch (...) {
        return false;
    }
}

bool parse_double(std::string_view text, double& out) {
    try {
        std::size_t pos = 0;
        out = std::stod(std::string{text}, &pos);
        return pos == text.size();
    } catch (...) {
        return false;
    }
}

bool fail_unknown_option(std::string_view arg, std::ostream& err) {
    err << "Unknown option: " << arg << '\n';
    err << "Use --help to see available options.\n";
    return false;
}

} // namespace cli_common
