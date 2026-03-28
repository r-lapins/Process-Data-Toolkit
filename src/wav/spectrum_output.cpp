#include "pdt/wav/spectrum_output.h"

#include <sstream>
#include <iomanip>

namespace pdt {

bool write_spectrum_report(std::ostream &out, const SpectrumReport &report)
{
    if (!out) {
        return false;
    }

    const auto& m = report.meta;

    out << "Input file   : " << m.input_path << '\n';
    out << "Sample rate  : " << m.sample_rate << " Hz\n";
    out << "Channels     : " << m.channels << '\n';
    out << "Samples      : " << m.total_samples << '\n';
    out << "From sample  : " << m.from << '\n';
    out << "Window size  : " << m.windowSize << '\n';
    out << "Window       : " << m.window << '\n';
    out << "Algorithm    : " << m.algorithm << '\n';
    out << "Threshold    : " << m.threshold << '\n';
    out << "Peak mode    : " << m.peak_mode << '\n';
    out << "Detected peaks: " << report.all_peaks.size()
        << " | showing top " << m.top << '\n';

    if (report.all_peaks.empty()) {
        out << "No peaks detected.\n";
        return static_cast<bool>(out);
    }

    for (std::size_t i = 0; i < report.top_peaks.size(); ++i) {
        const auto& peak = report.top_peaks[i];
        out << format_peak_line(peak, i + 1) << '\n';
    }

    return static_cast<bool>(out);
}

std::string format_peak_line(const Peak& peak, std::size_t display_index)
{
    std::ostringstream out;
    out << std::setfill(' ') << std::setw(3) << display_index << ". "
        << "f = " << std::fixed << std::setprecision(2) << peak.frequency
        << " Hz"
        << "    |X| = " << peak.magnitude
        << std::defaultfloat
        << "    bin = " << peak.index;
    return out.str();
}

bool write_spectrum_csv(std::ostream& out, const Spectrum& spectrum) {
    if (!out) {
        return false;
    }

    if (spectrum.frequencies.size() != spectrum.magnitudes.size()) {
        return false;
    }

    out << "frequency_hz,magnitude\n";

    for (std::size_t i = 0; i < spectrum.frequencies.size(); ++i) {
        out << spectrum.frequencies[i] << ',' << spectrum.magnitudes[i] << '\n';
    }

    return static_cast<bool>(out);
}

std::string to_string(const SpectrumReport &report)
{
    std::ostringstream out;
    if (!write_spectrum_report(out, report)) {
        return {};
    }
    return out.str();
}

} // namespace pdt