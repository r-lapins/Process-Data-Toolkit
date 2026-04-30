#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct rtlsdr_dev;

namespace pdt {

struct RtlSdrDeviceInfo {
    std::uint32_t index{};
    std::string vendor;
    std::string product;
    std::string serial;
};

struct RtlSdrConfig {
    std::uint32_t device_index{0};
    std::uint32_t center_frequency{100000000};
    std::uint32_t sample_rate{1024000};
    int tuner_gain_tenth_db{0};   // 0 = auto
    bool bias_tee{false};
};

class RtlSdrDevice {
  public:
    RtlSdrDevice() = default;
    ~RtlSdrDevice();

    RtlSdrDevice(const RtlSdrDevice&) = delete;
    RtlSdrDevice& operator=(const RtlSdrDevice&) = delete;

    RtlSdrDevice(RtlSdrDevice&& other) noexcept;
    RtlSdrDevice& operator=(RtlSdrDevice&& other) noexcept;

    static std::vector<RtlSdrDeviceInfo> enumerate();

    bool open(std::uint32_t index);
    void close();

    bool is_open() const noexcept;

    bool apply_config(const RtlSdrConfig& config);
    bool cancel_async();

    [[nodiscard]] rtlsdr_dev* native_handle() noexcept { return dev_; }
    [[nodiscard]] const rtlsdr_dev* native_handle() const noexcept { return dev_; }

  private:
    rtlsdr_dev* dev_ = nullptr;
};

} // namespace pdt
