#pragma once

#include "pdt/signal/dft.h"

#include <ostream>

namespace pdt {

// Writes spectrum data as CSV:
// frequency_hz,magnitude
bool write_spectrum_csv(std::ostream& out, const Spectrum& spectrum);

} // namespace pdt
