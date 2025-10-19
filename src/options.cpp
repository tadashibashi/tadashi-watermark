#include "options.h"
#include <iostream>
#include <thread>

static fs::path make_path(const char *filepath)
{
    auto ret = fs::path(filepath);
    if (ret.has_relative_path())
        ret = fs::current_path() / ret;
    return ret;
}

// Safely check if string begins with "--"
static bool is_option(const char *arg)
{
    return (arg && arg[0] == '-' && arg[1] == '-');
}

// Compare strings for equality
static bool equals(const char *a, const char *b)
{
    return strcmp(a, b) == 0;
}

bool options::parse_args(int argc, char *argv[])
{
    if (argc < 3) return false;

    int argi = 1;

    std::vector<fs::path> track_filenames{};
    std::vector<fs::path> output_filenames{};
    fs::path wmark_filename{};
    uint32_t wmark_offset_ms = 3'000u;
    uint32_t wmark_gap_ms = 10'000u;
    double wmark_volume = 1.0;
    uint32_t samplerate = 44100u;
    uint32_t bitrate = 128u;
    uint32_t threadpool_size = 8u;

    // Get track names
    while (argi < argc && argv[argi][0] != '-') {
        track_filenames.emplace_back(
            make_path(argv[argi]));
        ++argi;
    }

    // Get the watermark filename
    if (track_filenames.size() > 1) {
        wmark_filename = track_filenames.back();
        track_filenames.pop_back();
    } else {
        std::cerr << "Please include at least one track filename and the watermark filename\n";
        return false;
    }

    // Parse options
    while (argi < argc - 1) {
        const char *arg = argv[argi];
        const char *next = argv[argi + 1];
        if (is_option(arg)) {
            // Sample rate
            arg = &arg[2];
            if (equals(arg, "samplerate")) {
                if (is_option(next)) {
                    std::cerr << "--samplerate is missing an arg value\n";
                    return false;
                }

                samplerate = std::stoul(next);
                ++argi;
            } else if (equals(arg, "bitrate")) {
                if (is_option(next)) {
                    std::cerr << "--bitrate is missing an arg value\n";
                    return false;
                }

                bitrate = std::stoul(next);
                ++argi;
            } else if (equals(arg, "watermark-begin")) {
                if (is_option(next)) {
                    std::cerr << "--watermark-begin is missing an arg value\n";
                    return false;
                }

                wmark_offset_ms = std::stoul(next);
                ++argi;
            } else if (equals(arg, "watermark-gap")) {
                if (is_option(next)) {
                    std::cerr << "--watermark-gap is missing an arg value\n";
                    return false;
                }

                wmark_gap_ms = std::stoul(next);
                ++argi;
            }  else if (equals(arg, "watermark-volume")) {
                if (is_option(next)) {
                    std::cerr << "--watermark-volume is missing an arg value\n";
                    return false;
                }

                wmark_volume = std::stod(next);
                ++argi;
            } else if (equals(arg, "output")) {
                ++argi;
                while (argi < argc && !is_option(argv[argi])) {
                    output_filenames.emplace_back(make_path(argv[argi]));
                    ++argi;
                }
            } else if (equals(arg, "threads")) {
            	threadpool_size = std::stoul(next);
            	if (threadpool_size == 0)
            		threadpool_size = 1u;
                if (threadpool_size < std::thread::hardware_concurrency())
                    threadpool_size = std::thread::hardware_concurrency();
            	++argi;
            }
        }

        ++argi;
    }

    // Auto add output filenames if any are missing/less than the number of input names
    const size_t input_filename_count = track_filenames.size();
    while (input_filename_count > output_filenames.size()) {
        const auto &track_filename = track_filenames[output_filenames.size()];
        output_filenames.emplace_back(
            track_filename.parent_path() /
            track_filename.stem().concat("-prev.mp3")
        );
    }

    this->bitrate = bitrate;
    this->output_filenames.swap(output_filenames);
    this->samplerate = samplerate;
    this->track_filenames.swap(track_filenames);
    this->wmark_filename.swap(wmark_filename);
    this->wmark_gap_ms = wmark_gap_ms;
    this->wmark_offset_ms = wmark_offset_ms;
    this->wmark_volume = wmark_volume;
    this->threadpool_size = threadpool_size;

    return true;
}

const char *options::get_help() noexcept
{
	return R"(
tadashi_watermark track1 track2 track3 watermark.mp3 [options]

Description:
    Takes multiple tracks and stamps each one with a repeating watermark file, outputting mp3's for each.
    There must be at least one track filename, and the last file is the watermark audio file.

options:
    --samplerate
        output mp3 sample rate in hertz, default is 44100
    --bitrate
        output mp3 bitrate, default is 128
    --output
        output mp3 filenames, corresponds with each track arg, in corresponding order, number of outputs should equal
        the number of tracks (not including the watermark file). If less, the same name in the same directory + -preview
        will be added.
    --watermark-begin
        the time in integer milliseconds before starting the watermark
    --watermark-gap
        the time in integer milliseconds before the timestamp repeats
    --threads
    	the maximum number of threads to permit
    	the maximum number of threads allowed (default: 8, actual threads may be less based on hardware concurrency)
)";
}
