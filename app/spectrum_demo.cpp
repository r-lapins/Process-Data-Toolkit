#include "pdt/dft.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>

int main() {
    using namespace pdt;

    // Parametry sygnału
    const double fs = 1000.0;     // częstotliwość próbkowania [Hz]
    const double f0 = 50.0;       // częstotliwość sinusoidy [Hz]
    const std::size_t N = 1000;   // liczba próbek

    // Generacja sygnału x[n] = sin(2π f0 n / Fs)
    std::vector<double> signal;
    signal.reserve(N);

    for (std::size_t n = 0; n < N; ++n) {
        const double sample = std::sin(2.0 * std::numbers::pi_v<double> * f0 * static_cast<double>(n) / fs);
        signal.push_back(sample);
    }

    // Oblicz widmo
    const auto spectrum = compute_single_sided_spectrum(signal, fs);

    std::cout << "Single-sided spectrum (first 20 bins)\n";
    std::cout << "-------------------------------------\n";

    for (std::size_t i = 0; i < spectrum.frequencies.size(); ++i) {
        std::cout
            << "bin " << i
            << "  f=" << spectrum.frequencies[i]
            << " Hz"
            << "  |X|=" << spectrum.magnitudes[i]
            << "\n";
    }

    // Znajdź największy pik
    const auto max_it = std::ranges::max_element(spectrum.magnitudes);

    const auto max_index = static_cast<std::size_t>(std::distance(spectrum.magnitudes.begin(), max_it));

    std::cout << "\nPeak detected:\n";
    std::cout << "frequency = " << spectrum.frequencies[max_index] << " Hz\n";
    std::cout << "magnitude = " << spectrum.magnitudes[max_index] << "\n";

    return 0;
}
