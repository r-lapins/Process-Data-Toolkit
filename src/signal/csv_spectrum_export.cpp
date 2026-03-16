#include "pdt/signal/csv_spectrum_export.h"

namespace pdt {

bool write_spectrum_csv(std::ostream& out, const Spectrum& spectrum) {
    if (!out) {
        return false;
    }

    if (spectrum.frequencies.size() != spectrum.magnitudes.size()) {
        return false;
    }

    out << "frequency_hz,magnitude\n";

    for (std::size_t i = 0; i < spectrum.frequencies.size(); ++i) {
        out << spectrum.frequencies[i] << ',' << spectrum.magnitudes[i] << '\n';
    }

    return static_cast<bool>(out);
}

} // namespace pdt
