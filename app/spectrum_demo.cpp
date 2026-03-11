#include "pdt/dft.h"
#include "pdt/peak_detection.h"
#include "pdt/wav_reader.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    using namespace pdt;

    if (argc < 2) {
        std::cerr << "Usage: spectrum_demo <input.wav>\n";
        return 1;
    }

    const auto wav = read_wav_pcm16_mono(std::filesystem::path(argv[1]));
    if (!wav) {
        std::cerr << "Failed to read WAV file or unsupported format.\n";
        return 1;
    }

    std::cout << "Loaded WAV file\n";
    std::cout << "sample_rate = " << wav->sample_rate << " Hz\n";
    std::cout << "channels    = " << wav->channels << "\n";
    std::cout << "samples     = " << wav->samples.size() << "\n";

           // Analyze only a fragment to keep DFT runtime reasonable.
           // DFT is O(N^2), so do not feed the entire file if it is large.
    const std::size_t start = 300;
    const std::size_t end = std::min<std::size_t>(10000, wav->samples.size());

    if (start >= end) {
        std::cerr << "Selected sample range is invalid.\n";
        return 1;
    }

    std::vector<double> segment(wav->samples.begin() + static_cast<std::ptrdiff_t>(start),
                                wav->samples.begin() + static_cast<std::ptrdiff_t>(end));

    std::cout << "Analyzed range: [" << start << ", " << end << ")\n";
    std::cout << "segment size   = " << segment.size() << "\n";

    const auto spectrum = compute_single_sided_spectrum(segment, wav->sample_rate);

    const auto dominant_peaks = detect_dominant_peaks(
        spectrum,
        0.4,
        PeakDetectionMode::LocalMaxima,
        10
        );

    std::cout << "\nDominant peaks\n";
    std::cout << "-------------------------------------\n";

    for (const auto& peak : dominant_peaks) {
        std::cout
            << "index=" << peak.index
            << "  f=" << peak.frequency
            << " Hz"
            << "  |X|=" << peak.magnitude
            << '\n';
    }

    return 0;
}
