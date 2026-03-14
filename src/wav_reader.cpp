#include "pdt/wav_reader.h"

#include <array>
#include <cstring>
#include <fstream>

namespace pdt {
namespace {

struct FmtChunk {
    std::uint16_t audio_format{};
    std::uint16_t num_channels{};
    std::uint32_t sample_rate{};
    std::uint32_t byte_rate{};
    std::uint16_t block_align{};
    std::uint16_t bits_per_sample{};
};

bool read_exact(std::istream& in, char* buffer, std::streamsize size) {
    in.read(buffer, size);
    return static_cast<bool>(in);
}

template <typename T>
bool read_value(std::istream& in, T& value) {
    static_assert(std::is_trivially_copyable_v<T>);
    return read_exact(in, reinterpret_cast<char*>(&value), static_cast<std::streamsize>(sizeof(T)));
}

bool read_fourcc(std::istream& in, std::array<char, 4>& fourcc) {
    return read_exact(in, fourcc.data(), 4);
}

bool fourcc_equals(const std::array<char,4>& a, const std::array<char,4>& b) {
    return a == b;
}

bool skip_bytes(std::istream& in, std::uint32_t size) {
    in.seekg(size, std::ios::cur);
    return static_cast<bool>(in);
}

} // namespace

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
std::optional<WavData> read_wav_pcm16_mono(const std::filesystem::path& path) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        return std::nullopt;
    }

    // RIFF header
    std::array<char, 4> chunk_id{};
    std::uint32_t riff_chunk_size{};
    std::array<char, 4> format{};

    if (!read_fourcc(in, chunk_id) || !read_value(in, riff_chunk_size) || !read_fourcc(in, format)) {
        return std::nullopt;
    }

    if (chunk_id != std::array{'R','I','F','F'} || format != std::array{'W','A','V','E'}) {
        return std::nullopt;
    }

    bool fmt_found = false;
    bool data_found = false;

    FmtChunk fmt{};
    std::vector<double> samples;

    // Read chunks until EOF or until fmt+data are found
    while (in && (!fmt_found || !data_found)) {
        std::array<char, 4> subchunk_id{};
        std::uint32_t subchunk_size{};

        if (!read_fourcc(in, subchunk_id)) {
            break;
        }

        if (!read_value(in, subchunk_size)) {
            return std::nullopt;
        }

        if (subchunk_id == std::array{'f', 'm', 't', ' '}) {
            if (subchunk_size < 16) {
                return std::nullopt;
            }

            if (!read_value(in, fmt.audio_format) ||
                !read_value(in, fmt.num_channels) ||
                !read_value(in, fmt.sample_rate) ||
                !read_value(in, fmt.byte_rate) ||
                !read_value(in, fmt.block_align) ||
                !read_value(in, fmt.bits_per_sample)) {
                return std::nullopt;
            }

            // Skip any extra fmt bytes beyond the PCM base header
            if (subchunk_size > 16) {
                if (!skip_bytes(in, subchunk_size - 16)) {
                    return std::nullopt;
                }
            }

            fmt_found = true;
        }
        else if (subchunk_id == std::array{'d', 'a', 't', 'a'}) {
            if (!fmt_found) {
                // In standard WAV files fmt usually comes first.
                // For simplicity, require fmt before data.
                return std::nullopt;
            }

            // Supported format:
            // PCM, mono, 16-bit
            if (fmt.audio_format != 1 || fmt.num_channels != 1 || fmt.bits_per_sample != 16) {
                return std::nullopt;
            }

            const std::uint32_t bytes_per_sample = fmt.bits_per_sample / 8;
            if (bytes_per_sample == 0 || fmt.block_align == 0) {
                return std::nullopt;
            }

            if (subchunk_size % bytes_per_sample != 0) {
                return std::nullopt;
            }

            const std::size_t sample_count = subchunk_size / bytes_per_sample;
            samples.reserve(sample_count);

            for (std::size_t i = 0; i < sample_count; ++i) {
                std::int16_t raw_sample{};
                if (!read_value(in, raw_sample)) {
                    return std::nullopt;
                }

                // Normalize signed 16-bit PCM to [-1.0, 1.0]
                constexpr double scale = 32768.0;
                samples.push_back(static_cast<double>(raw_sample) / scale);
            }

            data_found = true;
        }
        else {
            // Skip unknown chunks
            if (!skip_bytes(in, subchunk_size)) {
                return std::nullopt;
            }
        }

        // Chunks are word-aligned; odd-sized chunks have one padding byte
        if (subchunk_size % 2 != 0) {
            if (!skip_bytes(in, 1)) {
                return std::nullopt;
            }
        }
    }

    if (!fmt_found || !data_found) {
        return std::nullopt;
    }

    return WavData{
        .sample_rate = fmt.sample_rate,
        .channels = fmt.num_channels,
        .samples = std::move(samples)
    };
}

} // namespace pdt
