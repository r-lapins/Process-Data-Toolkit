#pragma once

#include "pdt/compute/ifft_backend.h"
#include "pdt/pipeline/spectrum_analysis_types.h"
#include "pdt/io/rtlsdr/iq_frame.h"

namespace pdt {

class IqSpectrumEngine {
  public:
    explicit IqSpectrumEngine(IFftBackend& backend);

    SpectrumAnalysisResult process(const IqFrame& frame,
                             const SpectrumAnalysisOptions& options) const;

  private:
    IFftBackend& backend_;
};

} // namespace pdt
