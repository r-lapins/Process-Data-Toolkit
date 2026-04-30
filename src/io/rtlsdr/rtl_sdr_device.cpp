#include "pdt/io/rtlsdr/rtl_sdr_device.h"

#include <rtl-sdr.h>

#include <array>
#include <utility>

namespace pdt {

RtlSdrDevice::~RtlSdrDevice()
{
    close();
}

RtlSdrDevice::RtlSdrDevice(RtlSdrDevice&& other) noexcept
    : dev_(std::exchange(other.dev_, nullptr))
{
}

RtlSdrDevice& RtlSdrDevice::operator=(RtlSdrDevice&& other) noexcept
{
    if (this != &other) {
        close();
        dev_ = std::exchange(other.dev_, nullptr);
    }
    return *this;
}

std::vector<RtlSdrDeviceInfo> RtlSdrDevice::enumerate()
{
    std::vector<RtlSdrDeviceInfo> result;

    const auto count = rtlsdr_get_device_count();
    result.reserve(count);

    for (std::uint32_t i = 0; i < count; ++i) {
        std::array<char, 256> manufacturer{};
        std::array<char, 256> product{};
        std::array<char, 256> serial{};

        rtlsdr_get_device_usb_strings(i, manufacturer.data(), product.data(), serial.data());

        result.push_back(RtlSdrDeviceInfo{.index = i,
                                          .vendor = manufacturer.data(),
                                          .product = product.data(),
                                          .serial = serial.data()
        });
    }

    return result;
}

bool RtlSdrDevice::open(std::uint32_t index)
{
    close();
    return rtlsdr_open(&dev_, index) == 0;
}

void RtlSdrDevice::close()
{
    if (dev_ != nullptr) {
        rtlsdr_close(dev_);
        dev_ = nullptr;
    }
}

bool RtlSdrDevice::is_open() const noexcept
{
    return dev_ != nullptr;
}

bool RtlSdrDevice::apply_config(const RtlSdrConfig& config)
{
    if (dev_ == nullptr) { return false; }

    if (rtlsdr_set_center_freq(dev_, config.center_frequency) != 0) { return false; }
    if (rtlsdr_set_sample_rate(dev_, config.sample_rate) != 0) { return false; }

    if (config.tuner_gain_tenth_db == 0) {
        if (rtlsdr_set_tuner_gain_mode(dev_, 0) != 0) { return false; }
    } else {
        if (rtlsdr_set_tuner_gain_mode(dev_, 1) != 0) { return false; }
        if (rtlsdr_set_tuner_gain(dev_, config.tuner_gain_tenth_db) != 0) { return false; }
    }

#ifdef RTLSDR_API_VERSION
    rtlsdr_set_bias_tee(dev_, config.bias_tee ? 1 : 0);
#endif

    if (rtlsdr_reset_buffer(dev_) != 0) { return false; }

    return true;
}

bool RtlSdrDevice::cancel_async()
{
    if (dev_ == nullptr) { return false; }
    return rtlsdr_cancel_async(dev_) == 0;
}

} // namespace pdt