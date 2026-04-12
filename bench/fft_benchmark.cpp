#include "pdt/compute/cpu_fft_backend.h"
#include "pdt/compute/cuda_fft_backend.h"
#include "pdt/dsp/dft.h"

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <vector>

using benchmark_clock = std::chrono::steady_clock;

std::vector<double> make_signal(std::size_t N) {
    std::vector<double> signal;
    signal.reserve(N);

    const double fs = 48000.0;
    const double f1 = 1000.0;
    const double f2 = 3500.0;
    const double f3 = 7200.0;

    for (std::size_t n = 0; n < N; ++n) {
        const double t = static_cast<double>(n) / fs;
        const double sample =
            (1.0 * std::sin(2.0 * std::numbers::pi_v<double> * f1 * t)) +
            (0.5 * std::sin(2.0 * std::numbers::pi_v<double> * f2 * t)) +
            (0.25 * std::sin(2.0 * std::numbers::pi_v<double> * f3 * t));
        signal.push_back(sample);
    }

    return signal;
}

template <typename Backend>
double benchmark_backend(Backend& backend, const std::vector<double>& signal, double sample_rate, pdt::WindowType window, int iterations) {
    const auto start = benchmark_clock::now();

    for (int i = 0; i < iterations; ++i) {
        auto result = backend.compute_spectrum(signal, sample_rate, window);
        (void)result;
    }

    const auto end = benchmark_clock::now();
    const std::chrono::duration<double, std::milli> elapsed = end - start;

    return elapsed.count() / static_cast<double>(iterations);
}

double benchmark_dft(const std::vector<double>& signal) {
    const auto start = benchmark_clock::now();

    auto result = pdt::compute_dft(signal);

    const auto end = benchmark_clock::now();

    std::chrono::duration<double, std::milli> elapsed = end - start;
    return elapsed.count();
}

int main() {
    constexpr double sample_rate = 48000.0;
    constexpr int iterations = 20;

    const std::vector<std::size_t> sizes = {
        512, 1024, 2048, 4096, 8192, 16384, 32768
    };

    pdt::CpuFftBackend cpu_backend;
    pdt::CudaFftBackend cuda_backend;

    constexpr int w_n = 8;
    constexpr int w_time = 12;
    constexpr int w_speed = 10;

    auto print_cell = [](std::optional<double> value, int width) {
        if (value.has_value()) {
            std::cout << std::setw(width)
            << std::fixed << std::setprecision(2)
            << *value;
        } else {
            std::cout << std::setw(width) << "-";
        }
    };

    std::cout << std::right
              << std::setw(w_n)     << "N"
              << std::setw(w_time)  << "DFT [ms]"
              << std::setw(w_time)  << "CPU [ms]"
              << std::setw(w_time)  << "CUDA [ms]"
              << std::setw(w_speed) << "Speedup"
              << '\n';
    std::cout << std::string((w_n + 3 * w_time) + w_speed, '-') << '\n';

    for (const auto N : sizes) {
        const auto signal = make_signal(N);

        const double cpu_ms = benchmark_backend(cpu_backend, signal, sample_rate, pdt::WindowType::Hann, iterations);
        const double cuda_ms = benchmark_backend(cuda_backend, signal, sample_rate, pdt::WindowType::Hann, iterations);

        std::optional<double> dft_ms;
        if (N == 512 || N == 1024) { dft_ms = benchmark_dft(signal); }

        const double speedup = cpu_ms / cuda_ms;

        std::cout << std::setw(w_n) << N;
        print_cell(dft_ms, w_time);
        print_cell(cpu_ms, w_time);
        print_cell(cuda_ms, w_time);
        std::cout << std::setw(w_speed)
                  << std::fixed << std::setprecision(2)
                  << speedup
                  << '\n';
    }

    return 0;
}
