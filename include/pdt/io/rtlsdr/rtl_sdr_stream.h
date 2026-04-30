#pragma once

#include "pdt/io/rtlsdr/iq_frame.h"
#include "pdt/io/rtlsdr/rtl_sdr_device.h"

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <thread>

namespace pdt {

class RtlSdrStream {
  public:
    using FrameCallback = std::function<void(IqFrame&&)>;

    RtlSdrStream() = default;
    ~RtlSdrStream();

    RtlSdrStream(const RtlSdrStream&) = delete;
    RtlSdrStream& operator=(const RtlSdrStream&) = delete;

    bool start(const RtlSdrConfig& config,
               std::size_t block_bytes,
               FrameCallback callback);

    void stop();
    bool is_running() const noexcept;

  private:
    static void rtl_callback(unsigned char* buf, std::uint32_t len, void* ctx);
    void handle_buffer(const unsigned char* buf, std::uint32_t len);

    RtlSdrDevice device_;
    FrameCallback callback_;
    std::thread worker_;
    std::atomic<bool> running_{false};
    std::uint32_t sample_rate_{};
    std::uint64_t sequence_{0};
    std::size_t block_bytes_{16384};
};

} // namespace pdt