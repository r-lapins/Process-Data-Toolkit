#include "pdt/io/rtlsdr/rtl_sdr_device.h"
#include "pdt/io/rtlsdr/rtl_sdr_stream.h"

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

int main()
{
    // === enumerate ===
    const auto devices = pdt::RtlSdrDevice::enumerate();

    if (devices.empty()) {
        std::cerr << "No RTL-SDR devices found\n";
        return 1;
    }

    std::cout << "Found devices:\n";
    for (const auto& d : devices) {
        std::cout << "  [" << d.index << "] "
                  << d.vendor << " " << d.product
                  << " SN: " << d.serial << '\n';
    }

    // === config ===
    pdt::RtlSdrConfig cfg;
    cfg.device_index = 0;
    cfg.center_frequency = 100'000'000; // 100 MHz
    cfg.sample_rate = 1'024'000;
    cfg.tuner_gain_tenth_db = 0;

    pdt::RtlSdrStream stream;

    std::atomic<int> frame_count = 0;
    std::atomic<bool> should_stop = false;

    const bool ok = stream.start(cfg, 16384,
                                 [&](pdt::IqFrame&& frame) {
                                     const int n = ++frame_count;

                                     std::cout << "Frame #" << n
                                               << " samples=" << frame.samples.size()
                                               << " sr=" << frame.sample_rate
                                               << '\n';

                                     if (n >= 10) {
                                         should_stop = true;
                                     }
                                 });

    if (!ok) {
        std::cerr << "Failed to start stream\n";
        return 1;
    }

    // wait until stream stops
    while (stream.is_running() && !should_stop) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    stream.stop();

    std::cout << "Done\n";
    return 0;
}