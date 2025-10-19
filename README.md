# Tadashi Watermark

A simple C++ 20 command line tool for printing watermarks onto an audio track demos.

Useful for batch automation in creating previews for audio in online stores.

## Features

- Mix file onto
- Fast, multithreaded for handling large file batches
- MP3, FLAC, and WAV supported for input files (see Requirements for vorbis and opus)
- Specify gap interval between repeats

Note: Only stereo MP3-encoded output is supported

## Requirements
- LAME installed locally
- CMake 3.20+
- libopusfile (if available, enables Opus input decoding)
- libvorbisfile (if available, enables Ogg Vorbis input decoding)

## How to use

```bash
tadashi_watermark track.wav track-2.flac track-etc.mp3 watermark.mp3 --output track_one.mp3 pizza.mp3 muzak-ish.mp3
```

### Arguments
There must be at least one input track filename plus the watermark file. The last filename listed will always be used as the watermark. These filename arguments must appear at the start right after the program name.

### Options
| Name                 | Description                                  |
|----------------------|----------------------------------------------|
| `--samplerate`       | Output sample rate in hertz (default: 44100) |
| `--bitrate`          | Output bitrate in kbps (default: 128)        |
| `--output`           | One or more output filenames. Each must correspond with each track filename in the same order. If less or no output filenames are provided, -prev will be added with the extension changed to .mp3. (e.g.: my_file.flac => my_file-prev.mp3)   |
| `--watermark-begin`  | The time in integer milliseconds before starting the watermark playback (default: 3000)   |
| `--watermark-gap`    | The gap time in integer milliseconds between the watermark  repeatitions (default: 10000) |
| `--watermark-volume` | The gap time in integer milliseconds between the watermark  repeatitions (default: 10000) |
| `--threads`          | The maximum number of threads allowed (default: 8, may be less based on hardware)         |
