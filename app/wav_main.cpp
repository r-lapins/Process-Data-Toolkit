#include "wav_cli_args.h"

#include "pdt/compute/cpu_fft_backend.h"
#include "pdt/compute/fft_size_policy.h"
#include "pdt/io/wav/wav_output.h"
#include "pdt/io/wav/wav_reader.h"
#include "pdt/pipeline/spectrum_engine.h"

#ifdef PDT_ENABLE_CUDA
#include "pdt/compute/cuda_fft_backend.h"
#include <memory>
#endif

#include <algorithm>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    using namespace pdt;
    using namespace wav_app;

    CliOptions options{};

    if (!parse_cli(argc, const_cast<const char* const*>(argv), options, std::cerr)) {
        print_help(std::cerr);
        return 1;
    }

    if (options.help_requested) {
        print_help(std::cout);
        return 0;
    }

    const auto wav = read_wav_pcm16_mono(options.input_path.c_str());
    if (!wav.has_value()) {
        std::cerr << "Failed to read WAV file or unsupported format.\n";
        return 1;
    }

    if (options.from >= wav->samples.size()) {
        std::cerr << "Start sample is out of range.\n";
        return 1;
    }

    const std::size_t available_samples = wav->samples.size() - options.from;

    if (options.print_fft_sizes) {
        std::vector<std::size_t> sizes;

        if (options.algorithm == pdt::SpectrumAlgorithm::cuFft) {
            sizes = pdt::best_cufft_sizes(available_samples);
        } else {
            sizes = pdt::best_fft_sizes(available_samples);
        }

        std::cout << "Available samples from --from: " << available_samples << "\n";
        std::cout << "Recommended window sizes:\n";

        for (auto n : sizes) {
            if (n == options.windowSize) { std::cout << ">"; }
            std::cout << n;
            if (n == options.windowSize) { std::cout << "<"; }
            std::cout << "  ";
        }
        std::cout << "\n";

        return 0;
    }

    if (options.from + options.windowSize > wav->samples.size()) {
        std::cerr << "Last " << options.from + options.windowSize - wav->samples.size()
                  << " samples are out of range.\n";
        return 1;
    }

    const std::size_t segment_size = std::min(options.windowSize, available_samples);

    if (segment_size == 0) {
        std::cerr << "Selected segment is empty.\n";
        return 1;
    }

    std::vector<double> segment(
        wav->samples.begin() + static_cast<std::ptrdiff_t>(options.from),
        wav->samples.begin() + static_cast<std::ptrdiff_t>(options.from + segment_size)
        );

    // CPU / GPU
    std::unique_ptr<IFftBackend> backend;

    switch (options.algorithm) {
    case SpectrumAlgorithm::Auto:
    case SpectrumAlgorithm::Dft:
    case SpectrumAlgorithm::Fft:
        backend = std::make_unique<CpuFftBackend>();
        break;

    case SpectrumAlgorithm::cuFft:
        #ifdef PDT_ENABLE_CUDA
        backend = std::make_unique<CudaFftBackend>();
        #else
        std::cerr << "This build does not include CUDA support.\n";
        return 1;
        #endif
        break;
    }

    SpectrumEngine engine{*backend};
    // ~CPU / GPU

    SpectrumAnalysisOptions analysis_options{.sample_rate = static_cast<double>(wav->sample_rate),
                                             .window = options.use_window ? options.window : WindowType::None,
                                             .peak_mode = options.peak_mode,
                                             .threshold = options.threshold,
                                             .max_peaks = options.max_peaks
    };

    if (options.algorithm == SpectrumAlgorithm::cuFft && !is_cuda_supported_fft_size(segment.size())) {
        std::cerr << "cuFFT does not support selected window size: "
                  << segment.size() << '\n';
        return 1;
    }

    SpectrumAnalysisResult analysis;

    try {
        analysis = engine.process(segment, analysis_options);
    } catch (const std::exception& ex) {
        std::cerr << "Spectrum analysis failed: " << ex.what() << '\n';
        return 1;
    }

    pdt::SpectrumReport report{.analysis = analysis,
                               .meta = {.input_path = options.input_path,
                                        .sample_rate = static_cast<double>(wav->sample_rate),
                                        .channels = wav->channels,
                                        .total_samples = wav->samples.size(),
                                        .from = options.from,
                                        .windowSize = segment.size(),
                                        .window = options.window,
                                        .algorithm = analysis.algorithm,
                                        .peak_mode = options.peak_mode,
                                        .threshold = options.threshold,
                                        .max_peaks = options.max_peaks
                               }
    };

    if (!pdt::write_spectrum_report(std::cout, report)) {
        std::cerr << "Failed to write spectrum report.\n";
        return 1;
    }

    if (!options.output_csv_path.empty()) {
        std::ofstream out_file(options.output_csv_path);
        if (!out_file) {
            std::cerr << "Failed to export CSV: " << options.output_csv_path << '\n';
            return 1;
        }

        if (!write_spectrum_csv(out_file, report.analysis.spectrum)) {
            std::cerr << "Failed to write spectrum CSV.\n";
            return 1;
        }

        std::cout << "Spectrum exported to: " << options.output_csv_path << '\n';
    }

    if (!options.output_report_path.empty()) {
        std::ofstream report_file(options.output_report_path);
        if (!report_file) {
            std::cerr << "Failed to open report file: " << options.output_report_path << '\n';
            return 1;
        }

        if (!write_spectrum_report(report_file, report)) {
            std::cerr << "Failed to write spectrum report.\n";
            return 1;
        }

        std::cout << "Report exported to: " << options.output_report_path << '\n';
    }

    return 0;
}
