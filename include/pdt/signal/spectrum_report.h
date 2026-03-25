#pragma once

#include "pdt/signal/peak_detection.h"

#include <string>
#include <vector>

namespace pdt {

struct SpectrumMetadata {
    std::string input_path;
    double sample_rate{};
    std::size_t channels{};
    std::size_t total_samples{};
    std::size_t from{};
    std::size_t bins{};
    std::string window;
    std::string algorithm;
    double threshold{};
    std::string peak_mode;
    std::size_t top{};
};

struct SpectrumReport {
    Spectrum spectrum;
    std::vector<Peak> peaks;
    SpectrumMetadata meta;
};

std::string format_spectrum_report(const SpectrumReport& report);
bool export_spectrum_csv(const Spectrum& spectrum, const std::string& path);

} // namespace pdt