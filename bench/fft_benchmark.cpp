#include "pdt/wav/dft.h"
#include "pdt/wav/fft.h"

#include <chrono>
#include <cmath>
#include <iostream>
#include <numbers>
#include <vector>

using benchmark_clock = std::chrono::steady_clock;

std::vector<double> make_signal(std::size_t N) {
    std::vector<double> signal;
    signal.reserve(N);

    const double fs = 1024.0;
    const double f0 = 50.0;

    for (std::size_t n = 0; n < N; ++n) {
        const double sample =
            std::sin(2.0 * std::numbers::pi_v<double> * f0 * static_cast<double>(n) / fs);
        signal.push_back(sample);
    }

    return signal;
}

double benchmark_dft(const std::vector<double>& signal) {
    const auto start = benchmark_clock::now();

    auto result = pdt::compute_dft(signal);

    const auto end = benchmark_clock::now();

    std::chrono::duration<double, std::milli> elapsed = end - start;
    return elapsed.count();
}

double benchmark_fft(const std::vector<double>& signal) {
    const auto start = benchmark_clock::now();

    auto result = pdt::compute_fft(signal);

    const auto end = benchmark_clock::now();

    std::chrono::duration<double, std::milli> elapsed = end - start;
    return elapsed.count();
}

int main() {
    std::cout << "\nN,DFT(ms),FFT(ms)\n";

    const std::vector<std::size_t> sizes = {
        64, 128, 256, 512, 1024, 2048
    };

    for (auto N : sizes) {

        auto signal = make_signal(N);

        const double dft_time = benchmark_dft(signal);
        const double fft_time = benchmark_fft(signal);

        std::cout
            << N << ","
            << dft_time << ","
            << fft_time << "\n";
    }
}
