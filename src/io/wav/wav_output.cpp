#include "pdt/io/wav/wav_output.h"

#include <sstream>
#include <iomanip>

namespace pdt {

bool write_spectrum_report(std::ostream &out, const SpectrumReport &report) {
    if (!out) { return false; }

    const auto& m = report.meta;

    out << "Input file    : " << m.input_path << '\n';
    out << "Sample rate   : " << m.sample_rate << " Hz\n";
    out << "Channels      : " << m.channels << '\n';
    out << "Samples       : " << m.total_samples << '\n';
    out << "From sample   : " << m.from << '\n';
    out << "Window size   : " << m.windowSize << '\n';
    out << "Window        : " << to_string(m.window) << '\n';
    out << "Algorithm     : " << to_string(m.algorithm) << '\n';
    out << "Total time    : " << std::fixed << std::setprecision(1)
        << report.analysis.total_time_ms << " ms\n";
    out << "Threshold     : " << m.threshold << '\n';
    out << "Peak mode     : " << to_string(m.peak_mode) << '\n';
    out << "Detected peaks: " << report.analysis.all_peaks.size()
        << " | showing top " << m.top << '\n';

    if (report.analysis.all_peaks.empty()) {
        out << "No peaks detected.\n";
        return static_cast<bool>(out);
    }

    for (std::size_t i = 0; i < report.analysis.top_peaks.size(); ++i) {
        const auto& peak = report.analysis.top_peaks[i];
        out << format_peak_line(peak, i + 1) << '\n';
    }

    return static_cast<bool>(out);
}

std::string format_peak_line(const Peak& peak, std::size_t display_index) {
    std::ostringstream out;
    out << std::setfill(' ') << std::setw(4) << display_index << "."
        << " f = "      << std::fixed << std::setprecision(2) << std::setw(8) << peak.frequency << " Hz"
        << "  |X| = "   << std::setw(7) << peak.magnitude << std::defaultfloat
        << "  bin = "   << std::setw(6) << peak.index;
    return out.str();
}

bool write_spectrum_csv(std::ostream& out, const Spectrum& spectrum) {
    if (!out) { return false; }

    if (spectrum.frequencies.size() != spectrum.magnitudes.size()) { return false; }

    out << "frequency_hz,magnitude\n";

    for (std::size_t i = 0; i < spectrum.frequencies.size(); ++i) {
        out << spectrum.frequencies[i] << ',' << spectrum.magnitudes[i] << '\n';
    }

    return static_cast<bool>(out);
}

std::string to_string(const SpectrumReport &report)
{
    std::ostringstream out;
    if (!write_spectrum_report(out, report)) { return {}; }
    return out.str();
}

const char* to_string(pdt::WindowType type) {
    using enum pdt::WindowType;

    switch (type) {
    case Hann:    return "hann";
    case Hamming: return "hamming";
    case None:    return "none";
    }
    return "unknown";
}

const char* to_string(pdt::SpectrumAlgorithm algorithm) {
    using enum pdt::SpectrumAlgorithm;

    switch (algorithm) {
    case Dft:   return "dft";
    case Fft:   return "fft";
    case cuFft: return "cufft";
    case Auto:  return "auto";
        break;
    }
    return "unknown";
}

const char* to_string(pdt::PeakDetectionMode mode) {
    using enum pdt::PeakDetectionMode;

    switch (mode) {
    case ThresholdOnly: return "threshold";
    case LocalMaxima:   return "local_maxima";
    }
    return "unknown";
}

} // namespace pdt