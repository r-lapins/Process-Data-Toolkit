#include "pdt/wav/spectrum_output.h"

#include <cassert>
#include <sstream>

int main() {
    using namespace pdt;

    Spectrum spectrum{
        .frequencies = {0.0, 10.0, 20.0},
        .magnitudes = {1.0, 2.5, 0.75}
    };

    std::ostringstream out;
    const bool ok = write_spectrum_csv(out, spectrum);

    assert(ok);

    const std::string expected =
        "frequency_hz,magnitude\n"
        "0,1\n"
        "10,2.5\n"
        "20,0.75\n";

    assert(out.str() == expected);

    return 0;
}
