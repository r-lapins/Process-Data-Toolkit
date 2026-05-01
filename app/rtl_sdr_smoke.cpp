#include "pdt/io/rtlsdr/rtl_sdr_device.h"
#include "pdt/io/rtlsdr/rtl_sdr_stream.h"
#include "pdt/io/rtlsdr/iq_ring_buffer.h"
#include "pdt/pipeline/iq_spectrum_engine.h"

#ifdef PDT_ENABLE_CUDA
    #include "pdt/compute/cuda_fft_backend.h"
#else
    #include "pdt/compute/cpu_fft_backend.h"
#endif

#include <atomic>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <thread>

/*
 What it tests:
    1. Detection of RTL-SDR devices (enumerate)
    2. Opening and configuring the device:
        - centre frequency
        - sample rate
        - gain
    3. Asynchronous data reception (rtlsdr_read_async)
    4. Conversion of data from RTL-SDR format (uint8 interleaved IQ) to std::complex<float>
    5. Data buffering between threads (IqRingBuffer)
    6. Processing in a separate thread (worker):
        - FFT (CPU backend)
        - IQ spectrum calculation (full: -Fs/2 .. +Fs/2)
        - Peak detection
    7. Measurement of processing time (per frame)
    8. Pipeline stability in live mode
 */

int main()
{
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
    cfg.center_frequency = 91'000'000;
    cfg.sample_rate = 1'024'000;
    cfg.tuner_gain_tenth_db = 0;

    pdt::RtlSdrStream stream;
    pdt::IqRingBuffer buffer{8};

    std::atomic<bool> running = true;

    // === processing ===
    #ifdef PDT_ENABLE_CUDA
        pdt::CudaFftBackend backend;
    #else
        pdt::CpuFftBackend backend;
    #endif
    pdt::IqSpectrumEngine engine{backend};

    pdt::SpectrumAnalysisOptions options;
    options.window = pdt::WindowType::Hann;
    options.threshold = 0.5;
    options.max_peaks = 1;

    std::thread worker([&]() {
        while (running) {
            auto frame = buffer.wait_pop_latest();
            if (!frame) {
                continue;
            }

            const auto result = engine.process(*frame, options);

            if (!result.top_peaks.empty()) {
                std::cout << "[LIVE] frame = " << frame->sequence
                          << "; samples: " << frame->samples.size()
                          << "; peaks = " << result.top_peaks.size()
                          << "; time = " << std::fixed << std::setprecision(2)
                          << result.total_time_ms << " ms\n";

                for (std::size_t i = 0; i < result.top_peaks.size(); ++i) {
                    const auto& p = result.top_peaks[i];

                    std::cout
                        << "  #"    << std::setw(2) << (i + 1)
                        << " f="    << std::setw(10) << std::fixed << std::setprecision(2) << p.frequency << " Hz | "
                        << "mag="  << std::setw(8)  << std::fixed << std::setprecision(1) << p.magnitude
                        << '\n';
                }
                std::cout << '\n';
            }
        }
    });

    // === capture ===
    const bool ok = stream.start(cfg, 16384*32,
                                 [&](pdt::IqFrame frame) {
                                     buffer.push(std::move(frame));
                                 });

    if (!ok) {
        std::cerr << "Failed to start stream\n";
        running = false;
        worker.join();
        return 1;
    }

    // === run ~3s ===
    std::this_thread::sleep_for(std::chrono::seconds(1));

    running = false;
    buffer.stop();
    stream.stop();
    worker.join();

    std::cout << "Done\n";
}
