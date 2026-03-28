#pragma once

#include <string_view>
#include <cstddef>
#include <optional>
#include <ostream>

namespace common_cli {

class ArgReader {
  public:
    ArgReader(int argc, const char* const* argv)
        : argc_(argc), argv_(argv), index_(1) {}

    bool has_next() const {
        return index_ < argc_;
    }

    std::string_view next() {
        return std::string_view{argv_[index_++]};
    }

    std::optional<std::string_view> value() {
        if (index_ >= argc_) {
            return std::nullopt;
        }
        return std::string_view{argv_[index_++]};
    }

    bool has_remaining() const {
        return index_ < argc_;
    }

  private:
    int argc_;
    const char* const* argv_;
    int index_;
};

[[nodiscard]] bool is_option(std::string_view text);

[[nodiscard]] bool parse_size_t(std::string_view text, std::size_t& out);
[[nodiscard]] bool parse_double(std::string_view text, double& out);

bool fail_unknown_option(std::string_view arg, std::ostream& err);

} // namespace common_cli
