#include "pdt/signal/spectrum_report.h"
#include "pdt/signal/csv_spectrum_export.h"

#include <sstream>
#include <fstream>

namespace pdt {

std::string format_spectrum_report(const SpectrumReport& report) {
    std::ostringstream os;

    const auto& m = report.meta;

    os << "\nInput file   : " << m.input_path << '\n';
    os << "Sample rate  : " << m.sample_rate << " Hz\n";
    os << "Channels     : " << m.channels << '\n';
    os << "Samples      : " << m.total_samples << '\n';
    os << "From sample  : " << m.from << '\n';
    os << "Bins         : " << m.bins << '\n';
    os << "Window       : " << m.window << '\n';
    os << "Algorithm    : " << m.algorithm << '\n';
    os << "Threshold    : " << m.threshold << '\n';
    os << "Peak mode    : " << m.peak_mode << '\n';
    os << "Top peaks    : " << m.top << '\n';

    os << "\nDominant peaks\n";
    os << "-------------------------------------\n";

    if (report.peaks.empty()) {
        os << "No peaks detected.\n";
        return os.str();
    }

    for (std::size_t i = 0; i < report.peaks.size(); ++i) {
        const auto& peak = report.peaks[i];

        os << (i + 1) << ". "
           << "f = " << peak.frequency << " Hz"
           << "    |X| = " << peak.magnitude
           << "    (bin " << peak.index << ")\n";
    }

    return os.str();
}

bool export_spectrum_csv(const Spectrum& spectrum, const std::string& path) {
    std::ofstream file(path);
    if (!file) {
        return false;
    }

    return write_spectrum_csv(file, spectrum);
}

} // namespace pdt