#include "pdt/compute/cpu_fft_backend.h"
#include "pdt/compute/cuda_fft_backend.h"
#include "pdt/dsp/dft.h"

#include <chrono>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <optional>
#include <string>
#include <vector>

using benchmark_clock = std::chrono::steady_clock;

namespace {

// Separates cold-start latency (plan creation, allocations, JIT)
// from steady-state throughput (reused plan + buffers).
struct BackendTiming {
    double first_call_ms{};
    double steady_state_ms{};
};

std::vector<double> make_signal(std::size_t N)
{
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

// Measures first-call latency (no warmup).
// For CUDA this includes context init, plan creation and allocations.
template <typename Backend>
double measure_single_call(Backend& backend,
                           const std::vector<double>& signal,
                           double sample_rate,
                           pdt::WindowType window)
{
    const auto start = benchmark_clock::now();
    auto result = backend.compute_spectrum(signal, sample_rate, window);
    (void)result;
    const auto end = benchmark_clock::now();

    const std::chrono::duration<double, std::milli> elapsed = end - start;
    return elapsed.count();
}

template <typename Backend>
double measure_average_calls(Backend& backend,
                             const std::vector<double>& signal,
                             double sample_rate,
                             pdt::WindowType window,
                             int iterations)
{
    const auto start = benchmark_clock::now();

    for (int i = 0; i < iterations; ++i) {
        auto result = backend.compute_spectrum(signal, sample_rate, window);
        (void)result;
    }

    const auto end = benchmark_clock::now();
    const std::chrono::duration<double, std::milli> elapsed = end - start;

    return elapsed.count() / static_cast<double>(iterations);
}

// Full benchmark:
// 1) first-call measurement (cold start)
// 2) warmup (stabilize caches / GPU state)
// 3) steady-state measurement (real throughput)
template <typename Backend>
BackendTiming benchmark_backend(Backend& backend,
                                const std::vector<double>& signal,
                                double sample_rate,
                                pdt::WindowType window,
                                int warmup_iterations,
                                int measure_iterations)
{
    BackendTiming timing{};

    timing.first_call_ms = measure_single_call(backend, signal, sample_rate, window);

    // Warmup phase: ensures plan reuse path is active and avoids measuring initialization cost.
    for (int i = 0; i < warmup_iterations; ++i) {
        auto result = backend.compute_spectrum(signal, sample_rate, window);
        (void)result;
    }

    timing.steady_state_ms = measure_average_calls(backend, signal, sample_rate,
                                                   window, measure_iterations);

    return timing;
}

double benchmark_dft(const std::vector<double>& signal)
{
    const auto start = benchmark_clock::now();
    auto result = pdt::compute_dft(signal);
    (void)result;
    const auto end = benchmark_clock::now();

    const std::chrono::duration<double, std::milli> elapsed = end - start;
    return elapsed.count();
}

void print_cell(std::optional<double> value, int width)
{
    if (value.has_value()) {
        std::cout << std::setw(width)
        << std::fixed << std::setprecision(2)
        << *value;
    } else {
        std::cout << std::setw(width) << "-";
    }
}

} // namespace

int main()
{
    constexpr double sample_rate = 48000.0;
    constexpr int warmup_iterations = 5;
    constexpr int measure_iterations = 20;

    const std::vector<std::size_t> sizes = {
        512, 1024, 2048, 4096, 8192, 16384, 32768, 65536
    };

    constexpr int w_n = 8;
    constexpr int w_time = 14;
    constexpr int w_speed = 12;

    // Cold-start latency (user-perceived delay on first run / size change)
    std::cout << "\n=== First-call latency ===\n";
    std::cout << std::right
              << std::setw(w_n)     << "N"
              << std::setw(w_time)  << "DFT [ms]"
              << std::setw(w_time)  << "CPU [ms]"
              << std::setw(w_time)  << "CUDA [ms]"
              << std::setw(w_speed) << "Speedup"
              << '\n';
    std::cout << std::string(w_n + (3 * w_time) + w_speed, '-') << '\n';

    for (const auto N : sizes) {
        const auto signal = make_signal(N);

        // New backend per N -> forces cold-start for each size.
        // Important: plan reuse happens only within same instance + same N.
        pdt::CpuFftBackend cpu_backend;
        pdt::CudaFftBackend cuda_backend;

        const auto cpu = benchmark_backend(cpu_backend,
                                           signal,
                                           sample_rate,
                                           pdt::WindowType::Hann,
                                           warmup_iterations,
                                           measure_iterations);

        const auto cuda = benchmark_backend(cuda_backend,
                                            signal,
                                            sample_rate,
                                            pdt::WindowType::Hann,
                                            warmup_iterations,
                                            measure_iterations);

        std::optional<double> dft_ms;
        if (N == 512 || N == 1024) { dft_ms = benchmark_dft(signal); }

        const double first_speedup =
            (cuda.first_call_ms > 0.0) ? (cpu.first_call_ms / cuda.first_call_ms) : 0.0;

        std::cout << std::setw(w_n) << N;
        print_cell(dft_ms, w_time);
        print_cell(cpu.first_call_ms, w_time);
        print_cell(cuda.first_call_ms, w_time);
        print_cell(first_speedup, w_speed);
        std::cout << '\n';
    }

    // Steady-state throughput (relevant for streaming / live processing)
    std::cout << "\n=== Steady-state throughput ===\n";
    std::cout << std::right
              << std::setw(w_n)     << "N"
              << std::setw(w_time)  << "CPU [ms]"
              << std::setw(w_time)  << "CUDA [ms]"
              << std::setw(w_speed) << "Speedup"
              << '\n';
    std::cout << std::string(w_n + (2 * w_time) + w_speed, '-') << '\n';

    for (const auto N : sizes) {
        const auto signal = make_signal(N);

        pdt::CpuFftBackend cpu_backend;
        pdt::CudaFftBackend cuda_backend;

        const auto cpu = benchmark_backend(cpu_backend,
                                           signal,
                                           sample_rate,
                                           pdt::WindowType::Hann,
                                           warmup_iterations,
                                           measure_iterations);

        const auto cuda = benchmark_backend(cuda_backend,
                                            signal,
                                            sample_rate,
                                            pdt::WindowType::Hann,
                                            warmup_iterations,
                                            measure_iterations);

        const double steady_speedup =
            (cuda.steady_state_ms > 0.0) ? (cpu.steady_state_ms / cuda.steady_state_ms) : 0.0;

        std::cout << std::setw(w_n) << N;
        print_cell(cpu.steady_state_ms, w_time);
        print_cell(cuda.steady_state_ms, w_time);
        print_cell(steady_speedup, w_speed);
        std::cout << '\n';
    }

    return 0;
}
