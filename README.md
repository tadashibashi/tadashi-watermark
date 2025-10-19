# Tadashi Watermark

A simple C++ 20 command line tool for imprinting watermarks onto an audio track demos.

Useful for batch automation in creating previews for audio in online stores.

## Requirements
- LAME installed locally

## How to use

```bash
tadashi_watermark track.wav track-2.flac track-etc.mp3 watermark.mp3 --output track_one.mp3 pizza.mp3 muzak-ish.mp3

Description:
Takes one or more tracks and stamps each one with a repeating watermark file over a specified interval, outputting mp3's for each.
There must be at least one track filename, and the last filename listed will be used as the watermark audio file.

Options:
    --samplerate
        output mp3 sample rate in hertz (default: 44100)
    --bitrate
        output mp3 bitrate (default: 128)
    --output
        output mp3 filenames, corresponds with each track arg, in corresponding order, number of outputs should equal
        the number of tracks (not including the watermark file). If less, the same name in the same directory + -prev
        will be added and changed to .mp3 extension. (e.g.: my_file.flac => my_file-prev.mp3)
    --watermark-begin
        the time in integer milliseconds before starting the watermark playback (default: 3000)
    --watermark-gap
        the gap time in integer milliseconds between the watermark repeatitions (default: 10000)
    --threads
    	the maximum number of threads allowed (default: 8, actual threads may be less based on hardware concurrency)
```
