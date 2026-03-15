#include "spectrum_cli_args.h"

#include <cassert>
#include <sstream>
#include <string>

int main() {
    using namespace spectrum_app;

    {
        const char* argv[] = {"spectrum_cli", "--help"};
        CliOptions opt{};
        std::stringstream err;

        const bool ok = parse_cli(2, argv, opt, err);

        assert(ok);
        assert(opt.help_requested);
        assert(err.str().empty());
    }

    {
        const char* argv[] = {
            "spectrum_cli",
            "--window", "hamming",
            "--from", "128",
            "--bins", "2048",
            "--threshold", "0.75",
            "--mode", "threshold-only",
            "--top", "5",
            "--algorithm", "fft",
            "input.wav"
        };
        CliOptions opt{};
        std::stringstream err;

        const bool ok = parse_cli(static_cast<int>(sizeof(argv) / sizeof(argv[0])), argv, opt, err);

        assert(ok);
        assert(err.str().empty());

        assert(opt.input_path == "input.wav");
        assert(opt.window == pdt::WindowType::Hamming);
        assert(opt.use_window == true);
        assert(opt.from == 128);
        assert(opt.bins == 2048);
        assert(opt.threshold == 0.75);
        assert(opt.peak_mode == pdt::PeakDetectionMode::ThresholdOnly);
        assert(opt.top == 5);
        assert(opt.algorithm == SpectrumAlgorithm::Fft);
        assert(!opt.help_requested);
    }

    {
        const char* argv[] = {"spectrum_cli", "--window", "none", "input.wav"};
        CliOptions opt{};
        std::stringstream err;

        const bool ok = parse_cli(4, argv, opt, err);

        assert(ok);
        assert(opt.input_path == "input.wav");
        assert(opt.use_window == false);
        // window może zostać domyślny, ważne że use_window=false
        assert(opt.window == pdt::WindowType::Hann);
    }

    {
        const char* argv[] = {"spectrum_cli", "input.wav"};
        CliOptions opt{};
        std::stringstream err;

        const bool ok = parse_cli(2, argv, opt, err);

        assert(ok);
        assert(opt.input_path == "input.wav");

               // sprawdzenie domyślnych wartości
        assert(opt.window == pdt::WindowType::Hann);
        assert(opt.use_window == true);
        assert(opt.from == 0);
        assert(opt.bins == 1024);
        assert(opt.threshold == 0.4);
        assert(opt.peak_mode == pdt::PeakDetectionMode::LocalMaxima);
        assert(opt.top == 10);
        assert(opt.algorithm == SpectrumAlgorithm::Auto);
        assert(!opt.help_requested);
    }

    {
        const char* argv[] = {"spectrum_cli"};
        CliOptions opt{};
        std::stringstream err;

        const bool ok = parse_cli(1, argv, opt, err);

        assert(!ok);
        assert(err.str().find("Missing input WAV file.") != std::string::npos);
    }

    {
        const char* argv[] = {"spectrum_cli", "--unknown"};
        CliOptions opt{};
        std::stringstream err;

        const bool ok = parse_cli(2, argv, opt, err);

        assert(!ok);
        assert(err.str().find("Unknown option: --unknown") != std::string::npos);
    }

    {
        const char* argv[] = {"spectrum_cli", "--window"};
        CliOptions opt{};
        std::stringstream err;

        const bool ok = parse_cli(2, argv, opt, err);

        assert(!ok);
        assert(err.str().find("Missing value for --window") != std::string::npos);
    }

    {
        const char* argv[] = {"spectrum_cli", "--window", "blackman", "input.wav"};
        CliOptions opt{};
        std::stringstream err;

        const bool ok = parse_cli(4, argv, opt, err);

        assert(!ok);
        assert(err.str().find("Invalid value for --window: blackman") != std::string::npos);
    }

    {
        const char* argv[] = {"spectrum_cli", "--from", "-1", "input.wav"};
        CliOptions opt{};
        std::stringstream err;

        const bool ok = parse_cli(4, argv, opt, err);

        assert(!ok);
        assert(err.str().find("Invalid value for --from: -1") != std::string::npos);
    }

    {
        const char* argv[] = {"spectrum_cli", "--bins", "0", "input.wav"};
        CliOptions opt{};
        std::stringstream err;

        const bool ok = parse_cli(4, argv, opt, err);

        assert(!ok);
        assert(err.str().find("Invalid value for --bins: 0") != std::string::npos);
    }

    {
        const char* argv[] = {"spectrum_cli", "--threshold", "-0.1", "input.wav"};
        CliOptions opt{};
        std::stringstream err;

        const bool ok = parse_cli(4, argv, opt, err);

        assert(!ok);
        assert(err.str().find("Invalid value for --threshold: -0.1") != std::string::npos);
    }

    {
        const char* argv[] = {"spectrum_cli", "--threshold", "1.1", "input.wav"};
        CliOptions opt{};
        std::stringstream err;

        const bool ok = parse_cli(4, argv, opt, err);

        assert(!ok);
        assert(err.str().find("Invalid value for --threshold: 1.1") != std::string::npos);
    }

    {
        const char* argv[] = {"spectrum_cli", "--threshold", "abc", "input.wav"};
        CliOptions opt{};
        std::stringstream err;

        const bool ok = parse_cli(4, argv, opt, err);

        assert(!ok);
        assert(err.str().find("Invalid value for --threshold: abc") != std::string::npos);
    }

    {
        const char* argv[] = {"spectrum_cli", "--mode", "bad-mode", "input.wav"};
        CliOptions opt{};
        std::stringstream err;

        const bool ok = parse_cli(4, argv, opt, err);

        assert(!ok);
        assert(err.str().find("Invalid value for --mode: bad-mode") != std::string::npos);
    }

    {
        const char* argv[] = {"spectrum_cli", "--top", "0", "input.wav"};
        CliOptions opt{};
        std::stringstream err;

        const bool ok = parse_cli(4, argv, opt, err);

        assert(!ok);
        assert(err.str().find("Invalid value for --top: 0") != std::string::npos);
    }

    {
        const char* argv[] = {"spectrum_cli", "--algorithm", "gpu", "input.wav"};
        CliOptions opt{};
        std::stringstream err;

        const bool ok = parse_cli(4, argv, opt, err);

        assert(!ok);
        assert(err.str().find("Invalid value for --algorithm: gpu") != std::string::npos);
    }

    {
        const char* argv[] = {"spectrum_cli", "a.wav", "b.wav"};
        CliOptions opt{};
        std::stringstream err;

        const bool ok = parse_cli(3, argv, opt, err);

        assert(!ok);
        assert(err.str().find("Only one input WAV file may be provided.") != std::string::npos);
    }

    {
        assert(std::string{to_string(pdt::WindowType::Hann)} == "hann");
        assert(std::string{to_string(pdt::WindowType::Hamming)} == "hamming");

        assert(std::string{to_string(pdt::PeakDetectionMode::ThresholdOnly)} == "threshold-only");
        assert(std::string{to_string(pdt::PeakDetectionMode::LocalMaxima)} == "local-maxima");

        assert(std::string{to_string(SpectrumAlgorithm::Auto)} == "auto");
        assert(std::string{to_string(SpectrumAlgorithm::Dft)} == "dft");
        assert(std::string{to_string(SpectrumAlgorithm::Fft)} == "fft");
    }

    {
        std::ostringstream out;
        print_help(out);
        const std::string text = out.str();

        assert(text.find("Usage:") != std::string::npos);
        assert(text.find("spectrum_demo [options] <input.wav>") != std::string::npos);
        assert(text.find("--window <none|hann|hamming>") != std::string::npos);
        assert(text.find("--algorithm <auto|dft|fft>") != std::string::npos);
    }

    return 0;
}
