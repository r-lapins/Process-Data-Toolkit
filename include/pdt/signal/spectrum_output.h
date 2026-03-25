#pragma once

#include "spectrum.h"
#include "peak_detection.h"

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
    std::string window;
    std::string algorithm;
    double threshold{};
    std::string peak_mode;
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
std::string to_string(const SpectrumReport& report);

} // namespace pdt