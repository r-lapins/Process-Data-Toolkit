#pragma once

#include "peak_detection.h"
#include "spectrum.h"
#include "window.h"

#include <string>
#include <vector>
#include <ostream>
#include <cstddef>

namespace pdt {

struct SpectrumMetadata {
    std::string input_path;
    double sample_rate{};
    std::size_t channels{};
    std::size_t total_samples{};
    std::size_t from{};
    std::size_t windowSize{};
    pdt::WindowType window;
    pdt::SpectrumAlgorithm algorithm;
    pdt::PeakDetectionMode peak_mode;
    double threshold{};
    std::size_t top{};
};

struct SpectrumReport {
    Spectrum spectrum;
    std::vector<Peak> all_peaks;
    std::vector<Peak> top_peaks;    // dominant
    SpectrumMetadata meta;
};

bool write_spectrum_report(std::ostream& out, const SpectrumReport& report);
bool write_spectrum_csv(std::ostream& out, const Spectrum& spectrum);
std::string format_peak_line(const Peak& peak, std::size_t display_index);
std::string to_string(const SpectrumReport& report);
const char* to_string(pdt::WindowType type);
const char* to_string(pdt::PeakDetectionMode mode);
const char* to_string(pdt::SpectrumAlgorithm algorithm);

} // namespace pdt