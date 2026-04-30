#pragma once

#include "pdt/compute/ifft_backend.h"

#include <complex>
#include <memory>

struct cufftHandle_t;

namespace pdt {

class CudaFftBackend final : public IFftBackend {
  public:
    CudaFftBackend();
    ~CudaFftBackend() override;

    CudaFftBackend(const CudaFftBackend&) = delete;
    CudaFftBackend& operator=(const CudaFftBackend&) = delete;

    const char* name()  const noexcept override { return "CUDA"; }
    bool is_gpu()       const noexcept override { return true; }

    FftComputationResult compute_spectrum(std::span<const double> signal,
                                          double sample_rate,
                                          WindowType window) override;

    FftComputationResult compute_iq_spectrum(std::span<const std::complex<float>> iq,
                                             double sample_rate,
                                             WindowType window) override;

  private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace pdt
