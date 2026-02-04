# GBS2VGM v2.0 - Quick Start Guide

## For Windows Users

### Step 1: Extract the Release
Unzip `gbs2vgm_v2.0.zip` to a folder, e.g., `C:\gbs2vgm_v2.0\`

### Step 2: Prepare Your GBS Files
Place your GBS files (or 7z/ZIP archives containing GBS files) in a convenient location.

### Step 3: Convert
Open Command Prompt (cmd) or PowerShell and run:

```cmd
cd C:\gbs2vgm_v2.0\bin
gbs2vgm_batch.exe "C:\path\to\your\game.7z"
```

### Step 4: Get Your VGM Files
The output will be in:
- Folder: `C:\path\to\your\Game Name\` (individual VGM files)
- ZIP: `C:\path\to\your\Game Name.zip` (packaged VGM files)

---

## For Linux/macOS Users

### Step 1: Extract the Release
```bash
unzip gbs2vgm_v2.0.zip
cd gbs2vgm_v2.0
```

### Step 2: Build the Program
```bash
cd source
chmod +x build.sh
./build.sh
```

This will create `gbs2vgm_batch` executable in the `source` directory.

### Step 3: Convert
```bash
./gbs2vgm_batch "/path/to/your/game.7z"
```

### Step 4: Get Your VGM Files
The output will be in:
- Folder: `/path/to/your/Game Name/` (individual VGM files)
- ZIP: `/path/to/your/Game Name.zip` (packaged VGM files)

---

## Common Usage Examples

### Convert a 7z Archive
```bash
gbs2vgm_batch "Dragon Quest III.7z"
```

### Convert a ZIP Archive
```bash
gbs2vgm_batch "Tales of Phantasia.zip"
```

### Convert with Custom Sample Rate
```bash
gbs2vgm_batch -r 48000 "Game.7z"
```

### Convert with Custom Fadeout
```bash
gbs2vgm_batch -f 5 "Game.7z"  # 5 second fadeout
```

### Enable Debug Mode
```bash
gbs2vgm_batch -d "Game.7z"
```

---

## Understanding the Output

### File Structure
```
Game Name/
├── 01 Track Name.vgm
├── 02 Another Track.vgm
├── 03 Yet Another Track.vgm
└── ...

Game Name.zip (contains all VGM files)
```

### VGM File Naming
- Format: `NN Track Name.vgm`
- `NN` = Track number (01, 02, 03, ...)
- Track names come from M3U playlist or GBS metadata

### Loop Behavior
- **Non-looping tracks** (loop_count=1): Play once with fadeout
- **Looping tracks** (loop_count≥2): Loop at beginning, player controls loop count

---

## Troubleshooting

### "Failed to open GBS file"
- Check that the file path is correct
- Make sure the file is a valid GBS, 7z, or ZIP file
- Try using absolute paths instead of relative paths

### "Bank out of range" Warning
- This is usually harmless and doesn't affect conversion
- The warning appears for some GBS files but conversion continues normally

### "0 cycles processed"
- Some subsongs may be empty or invalid
- This is normal for certain GBS files
- The converter will skip these and continue

### Conversion is Slow
- Large GBS files with many subsongs take longer
- Enable debug mode (-d) to see progress
- Typical speed: 1-2 seconds per track

### Output Files are Large
- VGM files are uncompressed by default
- Use the ZIP output for smaller file sizes
- Typical sizes: 100KB-1MB per track

---

## Advanced Usage

### Using M3U Playlists

Create a `.m3u` file to control track order, titles, and loop behavior:

```m3u
# @TITLE       Game Name
# @COMPOSER    Composer Name
# @DATE        2000-01-01

game.gbs::GBS,0,Title Screen,1:30,,2
game.gbs::GBS,1,Stage 1,2:00,,10
game.gbs::GBS,2,Boss Battle,1:45,,5
```

Then convert:
```bash
gbs2vgm_batch "playlist.m3u"
```

### Batch Processing Multiple Files

Windows:
```cmd
for %f in (*.7z) do gbs2vgm_batch.exe "%f"
```

Linux/macOS:
```bash
for f in *.7z; do ./gbs2vgm_batch "$f"; done
```

---

## Testing Your VGM Files

### Recommended VGM Players
- **in_vgm** (Winamp plugin) - Windows
- **VGMPlay** - Cross-platform
- **foo_input_vgm** (foobar2000 plugin) - Windows
- **Audacious** with VGM plugin - Linux

### Verify Loop Points
1. Open VGM file in player
2. Enable loop mode
3. Check that it loops correctly at the beginning
4. Verify it advances to next track after configured loop count

---

## Getting Help

- Read the full documentation: `docs/README.md`
- Check the changelog: `CHANGELOG.md`
- Review example files: `examples/`
- Report issues: https://github.com/anthropics/claude-code/issues

---

## Tips for Best Results

1. **Use M3U playlists** for better control over track metadata and loop behavior
2. **Set appropriate loop counts**: 1 for jingles, 2-10 for music tracks
3. **Check output ZIP files** for easy distribution
4. **Test with multiple VGM players** to ensure compatibility
5. **Keep original GBS files** as backup

---

Enjoy converting your Game Boy music to VGM format!
