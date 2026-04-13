#pragma once

#include "pdt/compute/ifft_backend.h"
#include "pdt/dsp/spectrum.h"

#include <memory>

namespace pdt {

std::unique_ptr<IFftBackend> create_fft_backend(SpectrumAlgorithm algorithm);
bool is_gpu_algorithm(SpectrumAlgorithm algorithm) noexcept;
bool is_algorithm_available(SpectrumAlgorithm algorithm) noexcept;

} // namespace pdt
