#include "pdt/window.h"

#include <cassert>
#include <cmath>
#include <vector>

int main() {
    using namespace pdt;

    constexpr double eps = 1e-9;

    // 1. Empty window
    {
        const auto w = make_window(WindowType::Hann, 0);
        assert(w.empty());
    }

    // 2. Size 1 window
    {
        const auto w = make_window(WindowType::Hann, 1);
        assert(w.size() == 1);
        assert(std::abs(w[0] - 1.0) < eps);
    }

    // 3. Hann window basic properties
    {
        const auto w = make_window(WindowType::Hann, 8);

        assert(w.size() == 8);

        // Hann starts and ends at zero
        assert(std::abs(w.front() - 0.0) < eps);
        assert(std::abs(w.back() - 0.0) < eps);

        // Symmetry
        for (std::size_t i = 0; i < w.size(); ++i) {
            assert(std::abs(w[i] - w[w.size() - 1 - i]) < eps);
        }
    }

    // 4. Hamming window basic properties
    {
        const auto w = make_window(WindowType::Hamming, 8);

        assert(w.size() == 8);

               // Hamming does not go to zero at the edges
        assert(std::abs(w.front() - 0.08) < 1e-6);
        assert(std::abs(w.back() - 0.08) < 1e-6);

        // Symmetry
        for (std::size_t i = 0; i < w.size(); ++i) {
            assert(std::abs(w[i] - w[w.size() - 1 - i]) < eps);
        }
    }

    // 5. Apply explicit window
    {
        const std::vector<double> signal{1.0, 2.0, 3.0};
        const std::vector<double> window{0.0, 0.5, 1.0};

        const auto out = apply_window(signal, window);

        assert(out.size() == 3);
        assert(std::abs(out[0] - 0.0) < eps);
        assert(std::abs(out[1] - 1.0) < eps);
        assert(std::abs(out[2] - 3.0) < eps);
    }

    // 6. Size mismatch
    {
        const std::vector<double> signal{1.0, 2.0, 3.0};
        const std::vector<double> window{1.0, 1.0};

        const auto out = apply_window(signal, window);
        assert(out.empty());
    }

    // 7. Apply generated Hann window
    {
        const std::vector<double> signal(8, 1.0);
        const auto out = apply_window(signal, WindowType::Hann);

        assert(out.size() == 8);

        // Since signal is all ones, output should equal the window itself
        const auto w = make_window(WindowType::Hann, 8);
        for (std::size_t i = 0; i < out.size(); ++i) {
            assert(std::abs(out[i] - w[i]) < eps);
        }
    }

    return 0;
}
