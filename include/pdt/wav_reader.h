#pragma once

#include <cstdint>
#include <optional>
#include <vector>
#include <filesystem>

namespace pdt {

struct WavData {
    std::uint32_t sample_rate{};
    std::uint16_t channels{};
    std::vector<double> samples;
};

std::optional<WavData> read_wav_pcm16_mono(const std::filesystem::path& path);

} // namespace pdt

/*
 * WavData
 *
 * Holds decoded WAV metadata and normalized samples.
 *
 * samples are converted from signed 16-bit PCM to double in range [-1.0, 1.0].
 */

/*
 * read_wav_pcm16_mono
 *
 * Reads a WAV file from disk and supports only:
 * - RIFF/WAVE
 * - PCM (format code 1)
 * - 16-bit samples
 * - mono
 *
 * Returns std::nullopt on invalid/unsupported input.
 */
