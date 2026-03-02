#include "cli_args.h"
#include <cassert>
#include <sstream>

int main() {
    using namespace pdt_app;

    {
        const char* argv[] = {"pdt_cli", "--help"};
        CliOptions opt{};
        std::stringstream err;
        bool ok = parse_args(2, argv, opt, err);
        assert(ok);
        assert(opt.help);
    }

    {
        const char* argv[] = {"pdt_cli", "--in", "x.csv", "--sensor", "S1",
                              "--from", "2026-02-18T10:00:00", "--to", "2026-02-18T12:00:00"};
        CliOptions opt{};
        std::stringstream err;
        bool ok = parse_args(9, argv, opt, err);
        assert(ok);
        assert(opt.input_path == "x.csv");
        assert(opt.sensor && *opt.sensor == "S1");
        assert(opt.from.has_value());
        assert(opt.to.has_value());
    }

    {
        const char* argv[] = {"pdt_cli", "--from", "bad"};
        CliOptions opt{};
        std::stringstream err;
        bool ok = parse_args(3, argv, opt, err);
        assert(!ok);
    }

    {
        const char* argv[] = {"pdt_cli", "--unknown"};
        CliOptions opt{};
        std::stringstream err;
        bool ok = parse_args(2, argv, opt, err);
        assert(!ok);
    }

    return 0;
}
