#pragma once

#include <filesystem>
#include <vector>

namespace fs = std::filesystem;

struct options {
    std::vector<fs::path> track_filenames{};
    std::vector<fs::path> output_filenames{};
    fs::path wmark_filename = "";
    uint32_t wmark_offset_ms = 3'000u;
    uint32_t wmark_gap_ms = 10'000u;
    double wmark_volume = 1.0;
    uint32_t samplerate = 44100u;
    uint32_t bitrate = 128u;
    uint32_t threadpool_size = 8u;

    /// Parse arguments from the command line
    bool parse_args(int argc, char *argv[]);

    /// Get the console help text
    static const char *get_help() noexcept;
};