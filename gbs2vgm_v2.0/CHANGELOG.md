# GBS2VGM Changelog

## Version 2.0 (2026-02-04)

### Major Fixes

#### 1. Fixed GBS File Support for Different Memory Mappings
- **Problem**: GBS files with high load addresses (like Tales of Phantasia at 0x2c00) would crash with segmentation faults
- **Root Cause**:
  - Replayer code was placed at fixed address 0x4000, conflicting with GBS code
  - `mapper_map()` had size_t underflow bug when calculating bank offsets
  - `bank_get()` didn't check for NULL pointers
- **Solution**:
  - Calculate replayer code address dynamically after GBS code ends
  - Round up to next 256-byte boundary for safety
  - Add overflow checks in `mapper_map()` before subtraction
  - Add NULL pointer checks in `bank_get()`
- **Files Modified**:
  - `gbs.c` (lines 1136-1256): Dynamic replayer address calculation
  - `mapper.c` (lines 62-96): Overflow and NULL checks
- **Impact**: Tales of Phantasia and other high-load-address GBS files now convert successfully

#### 2. Fixed VGM Loop Point Calculation
- **Problem**:
  - Loop points were marked in the middle of songs
  - VGM players would loop forever without advancing to next track
  - File sizes were unnecessarily large (2x duration)
- **Root Cause**:
  - Misunderstanding of VGM loop point semantics
  - Loop point should be at song beginning, not middle
  - Was rendering 2x duration (intro + loop)
- **Solution**:
  - Mark loop point at ~0.1s after song start for looping tracks
  - Only render one complete duration
  - Set VGM header correctly:
    - `total_samples` = samples before loop point (~0.1s)
    - `loop_samples` = samples in loop section (almost entire song)
    - `loop_offset` = file position relative to 0x1C
- **Files Modified**:
  - `gbs2vgm_batch.c` (lines 288-323): Loop point marking logic
  - `vgm_writer.c` (lines 246-262): VGM header calculation
- **Impact**:
  - VGM players now correctly loop and advance to next track
  - File sizes reduced by ~50% (DQ1&2: 11MB→4.8MB, Tales: 18MB→7.8MB)
  - Behavior matches standard VGM files like Kirby's Dream Land

#### 3. Fixed Unknown Opcode Handling
- **Problem**: CPU would stop on unknown opcodes (0xE3, 0xED, 0xFD)
- **Root Cause**: Tales of Phantasia uses Z80 opcodes not in Game Boy's LR35902
- **Solution**: Modified `op_unknown()` to warn but continue execution
- **Files Modified**: `gbcpu.c` (lines 150-155)
- **Impact**: Tales of Phantasia now plays correctly

### New Features

#### 1. Cycle-Accurate Timing
- **Implementation**: Calculate sample delays based on CPU cycles instead of fixed time steps
- **Formula**: `samples = (cycles_elapsed * 44100) / 4194304`
- **Precision**: ~0.238μs (138,000x better than old 33ms method)
- **Benefits**:
  - No timing drift
  - Sample-accurate register write timestamps
  - More accurate VGM playback

#### 2. 60Hz Refresh Rate
- **Changed**: Time step from 33ms to 17ms
- **Reason**: Closer to Game Boy's actual frame rate
- **Impact**: More responsive register capture

#### 3. Automatic ZIP Packaging
- **Feature**: Output VGM files automatically packaged into ZIP archive
- **Naming**: `<Game Name>.zip`
- **Benefits**: Easier distribution and organization

### Bug Fixes

- Fixed duplicate track numbering in output filenames
- Fixed 7z extraction (missing semicolon in `archive_utils.c:117`)
- Fixed memory leaks in archive extraction
- Improved error messages and warnings

### Technical Improvements

- Added comprehensive debug logging
- Improved M3U parser robustness
- Better metadata extraction from filenames
- Enhanced GD3 tag generation

### Testing

Successfully tested with:
- Dragon Quest III (DQ3)
- Dragon Quest I & II (DQ1&2)
- Tales of Phantasia: Narikiri Dungeon
- Dragon Quest Monsters
- Kirby's Dream Land

### Performance

- Conversion speed: ~1-2 seconds per track
- Memory usage: <50MB typical
- File size reduction: ~50% compared to v1.0

### Documentation

- Complete README with usage examples
- M3U format specification
- VGM loop point explanation
- Build instructions for all platforms

---

## Version 1.0 (2026-02-03)

### Initial Release

- Basic GBS to VGM conversion
- M3U playlist support
- 7z/ZIP archive extraction
- Filename metadata parsing
- GD3 tag generation
- Batch processing

### Known Issues (Fixed in v2.0)

- Crashes on high-load-address GBS files
- Incorrect loop point calculation
- Files too large due to 2x rendering
- VGM players loop forever

---

## References

- VGM Format Specification v1.71
- libvgm loop point implementation
- MAME VGM logging code
- GME GBS parser
- gbsplay project
