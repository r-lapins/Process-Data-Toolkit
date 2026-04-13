#include "pdt/compute/cuda_fft_backend.h"
#include "pdt/compute/cpu_fft_backend.h"
#include "pdt/io/wav/wav_reader.h"
#include "pdt/pipeline/spectrum_engine.h"

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <iostream>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace {

using clock_type = std::chrono::steady_clock;

struct BenchOptions {
    std::filesystem::path input_path;
    std::vector<std::size_t> fft_sizes{1024, 2048, 4096, 8192, 16384, 32768, 65536};
    std::size_t from{0};
    int warmup_iters{3};
    int measure_iters{10};
    pdt::WindowType window{pdt::WindowType::Hann};
    double threshold{0.4};
    std::size_t top{10};
    pdt::PeakDetectionMode peak_mode{pdt::PeakDetectionMode::LocalMaxima};
};

void print_usage(const char* argv0)
{
    std::cout
        << "Usage:\n"
        << "  " << argv0 << " --in <file.wav> [options]\n\n"
        << "Options:\n"
        << "  --in <file.wav>                       Input WAV file (mono PCM16)\n"
        << "  --from <index>                        Start sample index (default: 0)\n"
        << "  --sizes <n1,n2,...>                   FFT sizes (default: 1024,2048,4096,8192,16384,32768,65536)\n"
        << "  --warmup <count>                      Warmup iterations per size (default: 3)\n"
        << "  --iters <count>                       Measured iterations per size (default: 10)\n"
        << "  --window <none|hann|hamming>          Window type (default: hann)\n"
        << "  --threshold <0..1>                    Peak threshold ratio (default: 0.4)\n"
        << "  --top <count>                         Top peaks count (default: 10)\n"
        << "  --mode <threshold-only|local-maxima>  Peak mode (default: local-maxima)\n"
        << "  --help                                Show this help\n";
}

std::optional<pdt::WindowType> parse_window(std::string_view s)
{
    if (s == "none")    { return pdt::WindowType::None; }
    if (s == "hann")    { return pdt::WindowType::Hann; }
    if (s == "hamming") { return pdt::WindowType::Hamming; }
    return std::nullopt;
}

std::optional<pdt::PeakDetectionMode> parse_peak_mode(std::string_view s)
{
    if (s == "threshold-only")  { return pdt::PeakDetectionMode::ThresholdOnly; }
    if (s == "local-maxima")    { return pdt::PeakDetectionMode::LocalMaxima; }
    return std::nullopt;
}

std::optional<std::vector<std::size_t>> parse_sizes(std::string_view s)
{
    std::vector<std::size_t> out;
    std::size_t pos = 0;

    while (pos < s.size()) {
        const std::size_t comma = s.find(',', pos);
        const std::string token = std::string{ s.substr(pos, comma == std::string_view::npos ? s.size() - pos : comma - pos) };

        if (token.empty()) { return std::nullopt; }

        try {
            const auto value = static_cast<std::size_t>(std::stoull(token));
            if (value == 0) { return std::nullopt; }
            out.push_back(value);
        } catch (const std::exception& e) {
        #ifndef NDEBUG // only in debug
            std::cerr << "parse_sizes error: " << e.what() << '\n';
        #endif
            return std::nullopt;
        }

        if (comma == std::string_view::npos) { break; }
        pos = comma + 1;
    }

    if (out.empty()) { return std::nullopt; }

    return out;
}

bool parse_args(int argc, char** argv, BenchOptions& options)
{
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];

        auto require_value = [&](const char* name) -> std::optional<std::string> {
            if (i + 1 >= argc) {
                std::cerr << "Missing value for " << name << '\n';
                return std::nullopt;
            }
            return std::string{argv[++i]};
        };

        if (arg == "--help") {
            print_usage(argv[0]);
            return false;
        }
        if (arg == "--in") {
            const auto v = require_value("--in");
            if (!v) { return false; }
            options.input_path = *v;
            continue;
        }
        if (arg == "--from") {
            const auto v = require_value("--from");
            if (!v) { return false; }
            try {
                options.from = static_cast<std::size_t>(std::stoull(*v));
            } catch (...) {
                std::cerr << "Invalid --from\n";
                return false;
            }
            continue;
        }
        if (arg == "--sizes") {
            const auto v = require_value("--sizes");
            if (!v) { return false; }
            const auto parsed = parse_sizes(*v);
            if (!parsed) {
                std::cerr << "Invalid --sizes\n";
                return false;
            }
            options.fft_sizes = *parsed;
            continue;
        }
        if (arg == "--warmup") {
            const auto v = require_value("--warmup");
            if (!v) { return false; }
            try {
                options.warmup_iters = std::stoi(*v);
            } catch (...) {
                std::cerr << "Invalid --warmup\n";
                return false;
            }
            if (options.warmup_iters < 0) {
                std::cerr << "--warmup must be >= 0\n";
                return false;
            }
            continue;
        }
        if (arg == "--iters") {
            const auto v = require_value("--iters");
            if (!v) { return false; }
            try {
                options.measure_iters = std::stoi(*v);
            } catch (...) {
                std::cerr << "Invalid --iters\n";
                return false;
            }
            if (options.measure_iters <= 0) {
                std::cerr << "--iters must be > 0\n";
                return false;
            }
            continue;
        }
        if (arg == "--window") {
            const auto v = require_value("--window");
            if (!v) { return false; }
            const auto parsed = parse_window(*v);
            if (!parsed) {
                std::cerr << "Invalid --window\n";
                return false;
            }
            options.window = *parsed;
            continue;
        }
        if (arg == "--threshold") {
            const auto v = require_value("--threshold");
            if (!v) { return false; }
            try {
                options.threshold = std::stod(*v);
            } catch (...) {
                std::cerr << "Invalid --threshold\n";
                return false;
            }
            if (options.threshold < 0.0 || options.threshold > 1.0) {
                std::cerr << "--threshold must be in [0, 1]\n";
                return false;
            }
            continue;
        }
        if (arg == "--top") {
            const auto v = require_value("--top");
            if (!v) { return false; }
            try {
                options.top = static_cast<std::size_t>(std::stoull(*v));
            } catch (...) {
                std::cerr << "Invalid --top\n";
                return false;
            }
            continue;
        }
        if (arg == "--mode") {
            const auto v = require_value("--mode");
            if (!v) { return false; }
            const auto parsed = parse_peak_mode(*v);
            if (!parsed) {
                std::cerr << "Invalid --mode\n";
                return false;
            }
            options.peak_mode = *parsed;
            continue;
        }

        std::cerr << "Unknown argument: " << arg << '\n';
        return false;
    }

    if (options.input_path.empty()) {
        std::cerr << "Missing --in <file.wav>\n";
        return false;
    }

    return true;
}

struct RunStats {
    double first_call_ms{};
    double avg_total_ms{};
    double avg_engine_reported_ms{};
    std::size_t peaks{};
};

RunStats benchmark_backend(
    pdt::IFftBackend& backend,
    std::span<const double> signal,
    double sample_rate,
    const BenchOptions& opt)
{
    pdt::SpectrumEngine engine{backend};

    pdt::SpectrumAnalysisOptions analysis_options{.sample_rate = sample_rate,
                                                  .window = opt.window,
                                                  .peak_mode = opt.peak_mode,
                                                  .threshold = opt.threshold,
                                                  .top = opt.top};

    RunStats stats{};

    {
        const auto t0 = clock_type::now();
        const auto result = engine.process(signal, analysis_options);
        const auto t1 = clock_type::now();

        stats.first_call_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
        stats.peaks = result.all_peaks.size();
    }

    for (int i = 0; i < opt.warmup_iters; ++i) {
        (void)engine.process(signal, analysis_options);
    }

    double total_engine_ms = 0.0;

    for (int i = 0; i < opt.measure_iters; ++i) {
        const auto result = engine.process(signal, analysis_options);

        total_engine_ms += result.total_time_ms;
        stats.peaks = result.all_peaks.size();
    }

    stats.avg_engine_reported_ms = total_engine_ms / static_cast<double>(opt.measure_iters);

    return stats;
}

void print_header()
{
    std::cout
        << std::left
        << std::setw(10) << "Backend"
        << std::setw(10) << "N"
        << std::setw(16) << "First [ms]"
        << std::setw(18) << "Avg engine [ms]"
        << std::setw(10) << "All peaks"
        << '\n';

    std::cout << std::string(80, '-') << '\n';
}

void print_row(const char* backend_name, std::size_t n, const RunStats& stats)
{
    std::cout
        << std::left
        << std::setw(10) << backend_name
        << std::setw(10) << n
        << std::setw(16) << std::fixed << std::setprecision(3) << stats.first_call_ms
        << std::setw(18) << std::fixed << std::setprecision(3) << stats.avg_engine_reported_ms
        << std::setw(10) << stats.peaks
        << '\n';
}

} // namespace

int main(int argc, char** argv)
{
    BenchOptions opt{};
    if (!parse_args(argc, argv, opt)) {
        return opt.input_path.empty() ? 1 : 0;
    }

    const auto wav = pdt::read_wav_pcm16_mono(opt.input_path);
    if (!wav) {
        std::cerr << "Failed to read WAV: " << opt.input_path << '\n';
        return 1;
    }

    if (opt.from >= wav->samples.size()) {
        std::cerr << "--from is out of range\n";
        return 1;
    }

    std::cout << "Input      : " << opt.input_path << '\n';
    std::cout << "Sample rate: " << wav->sample_rate << " Hz\n";
    std::cout << "Samples    : " << wav->samples.size() << '\n';
    std::cout << "From       : " << opt.from << '\n';
    std::cout << "Warmup     : " << opt.warmup_iters << '\n';
    std::cout << "Iterations : " << opt.measure_iters << '\n';
    std::cout << '\n';

    print_header();

    pdt::CpuFftBackend cpu_backend;
    pdt::CudaFftBackend cuda_backend;

    for (const std::size_t n : opt.fft_sizes) {
        if (opt.from + n > wav->samples.size()) { continue; }

        const std::span<const double> segment{wav->samples.data() + opt.from,
                                              n};

        try {
            const auto cpu_stats = benchmark_backend(cpu_backend,
                                                     segment,
                                                     static_cast<double>(wav->sample_rate),
                                                     opt);

            print_row(cpu_backend.name(), n, cpu_stats);
        } catch (const std::exception& ex) {
            std::cerr << "CPU benchmark failed for N=" << n << ": " << ex.what() << '\n';
        }

        try {
            const auto gpu_stats = benchmark_backend(cuda_backend,
                                                     segment,
                                                     static_cast<double>(wav->sample_rate),
                                                     opt);

            print_row(cuda_backend.name(), n, gpu_stats);
            std::cout << "\n";
        } catch (const std::exception& ex) {
            std::cerr << "CUDA benchmark failed for N=" << n << ": " << ex.what() << '\n';
        }
    }

    return 0;
}