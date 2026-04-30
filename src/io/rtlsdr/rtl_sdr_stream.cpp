#include "pdt/io/rtlsdr/rtl_sdr_stream.h"

#include <rtl-sdr.h>

namespace pdt {

RtlSdrStream::~RtlSdrStream()
{
    stop();
}

bool RtlSdrStream::start(const RtlSdrConfig& config,
                         std::size_t block_bytes,
                         FrameCallback callback)
{
    if (running_) { return false; }

    if (worker_.joinable()) {
        worker_.join();
    }

    if (!callback) { return false; }
    if (block_bytes == 0) { return false; }

    if (!device_.open(config.device_index)) { return false; }
    if (!device_.apply_config(config)) {
        device_.close();
        return false;
    }

    callback_ = std::move(callback);
    sample_rate_ = config.sample_rate;
    block_bytes_ = block_bytes;
    sequence_ = 0;
    running_ = true;

    worker_ = std::thread([this]() {
        rtlsdr_read_async(device_.native_handle(),
                          &RtlSdrStream::rtl_callback,
                          this,
                          0,
                          static_cast<uint32_t>(block_bytes_));
        running_ = false;
    });

    return true;
}

void RtlSdrStream::stop()
{
    if (!running_ && !worker_.joinable()) {
        device_.close();
        return;
    }

    device_.cancel_async();

    if (worker_.joinable()) {
        if (worker_.get_id() == std::this_thread::get_id()) {
            return;
        }

        worker_.join();
    }

    running_ = false;
    device_.close();
}

bool RtlSdrStream::is_running() const noexcept
{
    return running_;
}

void RtlSdrStream::rtl_callback(unsigned char* buf, std::uint32_t len, void* ctx)
{
    auto* self = static_cast<RtlSdrStream*>(ctx);
    if ((self == nullptr) || !self->running_) { return; }
    self->handle_buffer(buf, len);
}

void RtlSdrStream::handle_buffer(const unsigned char* buf, std::uint32_t len)
{
    if (!callback_ || len < 2) { return; }

    IqFrame frame;
    frame.sample_rate = sample_rate_;
    frame.sequence = sequence_++;
    frame.samples.reserve(len / 2);

    for (std::uint32_t i = 0; i + 1 < len; i += 2) {
        const float i_sample = (static_cast<float>(buf[i]) - 127.5F) / 127.5F;
        const float q_sample = (static_cast<float>(buf[i + 1]) - 127.5F) / 127.5F;
        frame.samples.emplace_back(i_sample, q_sample);
    }

    callback_(std::move(frame));
}

} // namespace pdt