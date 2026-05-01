#pragma once

#include "pdt/compute/ifft_backend.h"
#include "pdt/pipeline/spectrum_analysis_types.h"

#include <span>

namespace pdt {

class SpectrumEngine {
  public:
    explicit SpectrumEngine(IFftBackend& backend);

    SpectrumAnalysisResult process(std::span<const double> signal, const SpectrumAnalysisOptions& options) const;

  private:
    IFftBackend& backend_;
};

} // namespace pdt
