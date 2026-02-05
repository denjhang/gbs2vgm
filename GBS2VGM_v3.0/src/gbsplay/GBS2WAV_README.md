# gbs2wav - GBS to WAV Converter with M3U Support

A console program to convert Game Boy Sound (GBS) files to WAV format using M3U playlist files for track information.

## Features

- Reads M3U playlist files with track metadata
- Converts individual GBS subsongs to separate WAV files
- Automatically names output files based on track titles from M3U
- Supports custom sample rates and fadeout durations
- Handles GBS files with various memory layouts

## Building

```bash
cd gbsplay
make -f Makefile.gbs2wav
```

This will create the `gbs2wav` executable.

## Usage

```bash
./gbs2wav [options] <m3u-file>
```

### Options

- `-r <rate>` - Sample rate (default: 44100 Hz)
- `-f <seconds>` - Fadeout duration in seconds (default: 3)
- `-o <dir>` - Output directory (default: current directory)
- `-h` - Show help message

### Examples

Convert all tracks from an M3U file:
```bash
./gbs2wav ../dq3/CGB-BD3J-JPN.m3u
```

Convert with custom output directory:
```bash
./gbs2wav -o output_wav ../dq3/CGB-BD3J-JPN.m3u
```

Convert with custom sample rate and fadeout:
```bash
./gbs2wav -r 48000 -f 5 -o output_wav ../dq3/CGB-BD3J-JPN.m3u
```

## M3U Format

The M3U file should contain track information in the following format:

```
# @TITLE       Game Title
# @ARTIST      Artist Name
# @COMPOSER    Composer Name
# @DATE        Release Date

filename.gbs::GBS,subsong_number,Track Title,duration,,loop_count
```

Example:
```
# @TITLE       Dragon Warrior III
# @ARTIST      Chunsoft, Tose, Enix
# @COMPOSER    Koichi Sugiyama

CGB-BD3J-JPN.gbs::GBS,0,Overture ~ Roto's Theme,1:39,,1
CGB-BD3J-JPN.gbs::GBS,3,Intermezzo,0:50,,10
```

## Output

The program creates WAV files named according to the pattern:
```
<track_number> <track_title>.wav
```

For example:
- `01 Overture ~ Roto's Theme.wav`
- `02 Intermezzo.wav`
- `03 Prologue.wav`

## Testing

Test files are provided in the `dq3` folder:
```bash
./gbs2wav -o test_output ../dq3/CGB-BD3J-JPN.m3u
```

This will convert all 64 tracks from Dragon Warrior III to WAV files.

## Technical Notes

- The program uses the gbsplay library for GBS playback
- Supports GBS files with various memory layouts, including those with low load addresses
- The replayer code is placed at address 0x3000 to avoid conflicts with GBS data
- Output is 16-bit stereo PCM WAV format

## License

Licensed under GNU GPL v1 or, at your option, any later version.
