# GBS2VGM v2.0 - File Index

Welcome to GBS2VGM v2.0! This file helps you navigate the release package.

## 📖 Start Here

1. **New Users**: Read [QUICKSTART.md](QUICKSTART.md) for a quick introduction
2. **Detailed Info**: Read [README.md](README.md) for comprehensive documentation
3. **What's New**: Read [CHANGELOG.md](CHANGELOG.md) to see what changed in v2.0
4. **Release Info**: Read [RELEASE_SUMMARY.md](RELEASE_SUMMARY.md) for release details

## 📁 Folder Structure

### `/bin` - Executables
- `gbs2vgm_batch.exe` - **Windows executable** (ready to use)
- `build_batch.sh` - Build script for Linux/macOS

### `/source` - Source Code
Contains all C source files needed to build the program:
- `gbs2vgm_batch.c` - Main program
- `vgm_writer.c/h` - VGM file writer
- `m3u_parser.c/h` - M3U playlist parser
- `archive_utils.c/h` - 7z/ZIP extraction
- `gbs.c`, `gbcpu.c`, `gbhw.c`, `mapper.c` - GBS playback engine
- `build.sh` - Build script (run this on Linux/macOS)

### `/dependencies` - Libraries
- `/7z` - LZMA SDK for 7z extraction
- `/zlib` - zlib for ZIP compression

### `/examples` - Example Files
- `/input` - Example GBS archives to convert
  - Dragon Warrior I & II (40 KB)
  - Tales of Phantasia (28 KB)
- `/output` - Example converted VGM archives
  - Dragon Warrior I & II (4.8 MB, 49 tracks)
  - Tales of Phantasia (7.8 MB, 39 tracks)

### `/docs` - Documentation
- `README.md` - Detailed technical documentation
- `LICENCE` - GNU GPL v1 license
- `COPYRIGHT` - Copyright information

## 🚀 Quick Start

### Windows
```cmd
cd bin
gbs2vgm_batch.exe "path\to\your\game.7z"
```

### Linux/macOS
```bash
cd source
./build.sh
./gbs2vgm_batch "path/to/your/game.7z"
```

## 📚 Documentation Files

| File | Description |
|------|-------------|
| [README.md](README.md) | Main documentation with full usage guide |
| [QUICKSTART.md](QUICKSTART.md) | Quick start guide for beginners |
| [CHANGELOG.md](CHANGELOG.md) | Detailed version history and changes |
| [RELEASE_SUMMARY.md](RELEASE_SUMMARY.md) | Release information and features |
| [VERSION.txt](VERSION.txt) | Version information |
| [docs/README.md](docs/README.md) | Technical documentation |
| [docs/LICENCE](docs/LICENCE) | GNU GPL v1 license |
| [docs/COPYRIGHT](docs/COPYRIGHT) | Copyright information |

## 🎯 Common Tasks

### Convert a GBS Archive
```bash
gbs2vgm_batch "Game.7z"
```

### Convert with Custom Settings
```bash
gbs2vgm_batch -r 48000 -f 5 "Game.7z"
```

### Build from Source (Linux/macOS)
```bash
cd source
./build.sh
```

### Test with Examples
```bash
cd bin
gbs2vgm_batch.exe "../examples/input/Dragon Warrior I & II [Dragon Quest I+II] (1999-09-23)(Chunsoft)(Tose)(Enix)[GBC].7z"
```

## 🔧 Technical Details

- **Version**: 2.0
- **Release Date**: 2026-02-04
- **VGM Format**: v1.71
- **Timing Precision**: ~0.238μs (cycle-accurate)
- **Sample Rate**: 44100 Hz (default)
- **License**: GNU GPL v1 or later

## ✅ Tested Games

- Dragon Quest III (DQ3)
- Dragon Quest I & II (DQ1&2)
- Tales of Phantasia: Narikiri Dungeon
- Dragon Quest Monsters
- Kirby's Dream Land

## 🆘 Getting Help

1. Check [QUICKSTART.md](QUICKSTART.md) for common usage
2. Read [README.md](README.md) for detailed documentation
3. Review [CHANGELOG.md](CHANGELOG.md) for known issues
4. Report issues: https://github.com/anthropics/claude-code/issues

## 📦 Package Contents

- **Total Files**: 130+
- **Package Size**: 1.8 MB (compressed)
- **Extracted Size**: 15 MB
- **Source Files**: 25 C files
- **Documentation**: 8 files
- **Examples**: 4 files (2 input + 2 output)

## 🎵 Output Format

- **Format**: VGM v1.71
- **Chip**: Game Boy DMG
- **Registers**: 0xFF10-0xFF3F
- **Loop Points**: Correctly implemented
- **Metadata**: GD3 tags included

## 👥 Authors

- Claude Sonnet 4.5 (AI Assistant)
- Denjhang (Project Lead)

## 📄 License

GNU General Public License v1 or later

See [docs/LICENCE](docs/LICENCE) for full license text.

---

**Enjoy converting your Game Boy music to VGM format!**

For more information, start with [QUICKSTART.md](QUICKSTART.md) or [README.md](README.md).
