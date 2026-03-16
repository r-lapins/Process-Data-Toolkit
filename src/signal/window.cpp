#include "pdt/signal/window.h"

#include <cmath>
#include <numbers>

namespace pdt {

std::vector<double> make_window(WindowType type, std::size_t size) {
    std::vector<double> window;

    if (size == 0) {
        return window;
    }

    window.reserve(size);

    // For size == 1, return a neutral window.
    if (size == 1) {
        window.push_back(1.0);
        return window;
    }

    for (std::size_t n = 0; n < size; ++n) {
        const double phase =
            2.0 * std::numbers::pi_v<double> * static_cast<double>(n)
            / static_cast<double>(size - 1);

        double value = 1.0;

        switch (type) {
        case WindowType::Hann:
            value = 0.5 * (1.0 - std::cos(phase));
            break;

        case WindowType::Hamming:
            value = 0.54 - (0.46 * std::cos(phase));
            break;
        }

        window.push_back(value);
    }

    return window;
}

std::vector<double> apply_window(std::span<const double> signal,
                                 std::span<const double> window) {
    std::vector<double> output;

    if (signal.size() != window.size()) {
        return output;
    }

    output.reserve(signal.size());

    for (std::size_t i = 0; i < signal.size(); ++i) {
        output.push_back(signal[i] * window[i]);
    }

    return output;
}

std::vector<double> apply_window(std::span<const double> signal,
                                 WindowType type) {
    const auto window = make_window(type, signal.size());
    return apply_window(signal, window);
}

} // namespace pdt
