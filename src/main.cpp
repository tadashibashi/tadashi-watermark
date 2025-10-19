#include "errors.h"
#include "options.h"

#include <ctpl_stl.h>
#include <lame/lame.h>
#include <miniaudio.h>

#include <fstream>
#include <iostream>

#ifndef THREADPOOL_SIZE
#define THREADPOOL_SIZE 8
#endif

const uint32_t MP3_QUALITY_ALGORITHM = 2u;
const uint32_t MP3_CHANNELS = 2u;
const uint32_t MP3_BUFFER_SIZE = 65536u;

const ma_format PCM_MIX_FORMAT = ma_format_s16;
const uint32_t PCM_FRAMES_PER_ITERATION = 8192u;

ErrCode imprint_watermark(int id,
    const char *output_filename,
    const char *track_filename,
    const char *wmark_filename,
    const ma_decoder_config &decoder_config,
    uint32_t output_mp3_bitrate,
    uint32_t wmark_offset_ms,
    uint32_t wmark_gap_ms,
    double wmark_volume);

int main(int argc, char *argv[]) {
    // Init thread pool


    // Prepare arguments
    options opts{};
    if (!opts.parse_args(argc, argv)) {
        std::cout << options::get_help() << '\n';
        return -1;
    }

    const auto decoder_config = ma_decoder_config_init(
        PCM_MIX_FORMAT, MP3_CHANNELS, opts.samplerate);

    // Run each task in a thread pool
    auto threads = ctpl::thread_pool(opts.threadpool_size);

    size_t track_count = opts.track_filenames.size();
    for (size_t i = 0; i < track_count; ++i) {
        threads.push(imprint_watermark,
            opts.output_filenames[i].c_str(),
            opts.track_filenames[i].c_str(),
            opts.wmark_filename.c_str(),
            decoder_config,
            opts.bitrate,
            opts.wmark_offset_ms,
            opts.wmark_offset_ms,
            opts.wmark_volume);
    }

    threads.stop();
    return 0;
}

ErrCode imprint_watermark(int id,
    const char *output_filename,
    const char *track_filename,
    const char *wmark_filename,
    const ma_decoder_config &decoder_config,
    uint32_t output_mp3_bitrate,
    uint32_t wmark_offset_ms,
    uint32_t wmark_gap_ms,
    double wmark_volume)
{
    // ----- Initialize -----
    std::ofstream out_file(output_filename, std::ios::binary);
    if (!out_file.is_open()) {
        std::cerr << "Output file failed to open: \"" << output_filename << "\"\n";
        return ERR_FILE_READ;
    }

    ma_decoder track;
    if (ma_decoder_init_file(track_filename, &decoder_config, &track) != MA_SUCCESS) {
        std::cerr << "Failed to open track mp3 file!\n";
        return ERR_FILE_READ;
    }
    ma_decoder wmark;
    if (ma_decoder_init_file(wmark_filename, &decoder_config, &wmark) != MA_SUCCESS) {
        std::cerr << "Failed to open watermark mp3 file!\n";
        return ERR_FILE_READ;
    }

    lame_t lame = lame_init();
    if (!lame) {
        ma_decoder_uninit(&track);
        ma_decoder_uninit(&wmark);
        std::cerr << "Failed to init LAME.\n";
        return ERR_LAME_INIT;
    }

    lame_set_num_channels(lame, decoder_config.channels);
    lame_set_in_samplerate(lame, decoder_config.sampleRate);
    lame_set_brate(lame, output_mp3_bitrate);
    lame_set_quality(lame, MP3_QUALITY_ALGORITHM);
    lame_set_mode(lame, STEREO);

    if (lame_init_params(lame) == -1) {
        ma_decoder_uninit(&track);
        ma_decoder_uninit(&wmark);
        std::cerr << "Failed to init LAME parameters.\n";
        return ERR_LAME_INIT;
    }

    // ----- Read -----

    int16_t track_buf[PCM_FRAMES_PER_ITERATION * decoder_config.channels],
        wmark_buf[PCM_FRAMES_PER_ITERATION * decoder_config.channels];
    memset(track_buf, 0, sizeof(track_buf));
    memset(wmark_buf, 0, sizeof(wmark_buf));

    unsigned char mp3_buf[MP3_BUFFER_SIZE];

    bool wmark_is_playing = wmark_offset_ms == 0;
    uint64_t wmark_samples_until_play =
        (uint64_t)( (double)wmark_offset_ms * .001 * decoder_config.sampleRate );

    uint64_t current_track_frame = 0; // use this to track current time for watermarking
    
    while (true) {
        // TRACK DECODING
        ma_uint64 track_frames_read = 0;
        ma_result result = ma_decoder_read_pcm_frames(&track, track_buf, PCM_FRAMES_PER_ITERATION, &track_frames_read);

        if (track_frames_read == 0) break;

        // WATERMARK DECODING
        ma_uint64 wmark_frames_read = 0;
        if (wmark_is_playing) {
            result = ma_decoder_read_pcm_frames(&wmark, wmark_buf, PCM_FRAMES_PER_ITERATION, &wmark_frames_read);
            if (wmark_frames_read == 0) { // not super accurate, but ok for our purposes
                wmark_is_playing = false;
                wmark_samples_until_play = (uint64_t)(decoder_config.sampleRate * (wmark_gap_ms * .001));
                ma_decoder_seek_to_pcm_frame(&wmark, 0);
            }
        } else {
            if (wmark_samples_until_play == 0) {
                wmark_is_playing = true;
                wmark_samples_until_play = (uint64_t)(decoder_config.sampleRate * (wmark_gap_ms * .001));
                result = ma_decoder_seek_to_pcm_frame(&wmark, 0);
                if (result != MA_SUCCESS) {
                    std::cout << "failed to seek watermark back to start!\n";
                }
            }
        }

        // decrement wmark sample counter
        if (wmark_samples_until_play >= PCM_FRAMES_PER_ITERATION) {
            wmark_samples_until_play -= PCM_FRAMES_PER_ITERATION;
        } else {
            wmark_samples_until_play = 0;
        }

        // MIX BUFFERS
        for (uint32_t frame = 0; frame < wmark_frames_read; ++frame) {
            int base_index = frame * MP3_CHANNELS;
            for (uint32_t channel = 0; channel < MP3_CHANNELS; ++channel)
                track_buf[base_index + channel] += wmark_buf[base_index + channel] * wmark_volume;   
        }

        // Encode back to MP3
        auto mp3_bytes_encoded = lame_encode_buffer_interleaved(
            lame, track_buf, track_frames_read, mp3_buf, MP3_BUFFER_SIZE);
        if (mp3_bytes_encoded < 0) {
            lame_close(lame);
            ma_decoder_uninit(&track);
            ma_decoder_uninit(&wmark);

            std::cerr << "Lame failed to encode buffer\n";
            return ERR_LAME_ENCODE;
        }

        if (mp3_bytes_encoded > 0)
            out_file.write((const char *)mp3_buf, mp3_bytes_encoded);

        current_track_frame += track_frames_read;
    }

    auto flush_bytes = lame_encode_flush(lame, mp3_buf, MP3_BUFFER_SIZE);
    if (flush_bytes > 0) {
        out_file.write((const char *)mp3_buf, flush_bytes);
    }

    lame_close(lame);
    ma_decoder_uninit(&track);

    return ERR_OK;
}