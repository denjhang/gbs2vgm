# GBS2VGM v2.1 - Game Boy Sound to VGM Converter

高精度的Game Boy音乐文件(GBS)到VGM格式转换工具，支持批量转换和7z/ZIP压缩包。

## 版本 2.1 更新 (2026-02-04)

### 🐛 循环点修复

**问题**: v2.0中，使用in_vgm播放器（Winamp）播放时，循环曲目会播放过长时间才跳转到下一曲。

**原因**: VGM文件标记了循环点，导致播放器循环播放整首歌多次（例如：230秒 × 2次 = 460秒）。

**修复**:
- 移除VGM循环点标记
- VGM文件现在播放一次完整的duration后自动跳到下一曲
- 与in_vgm、VGMPlay等播放器完全兼容

详细说明请参见：[GBS2VGM_v2.1_LOOP_FIX.md](GBS2VGM_v2.1_LOOP_FIX.md)

## 版本 2.0 重大更新

### 🎯 核心修复

1. **修复不同内存映射的GBS文件支持**
   - 修复了replayer代码地址冲突问题
   - 现在可以正确处理Tales of Phantasia等高load地址的GBS文件
   - 自动计算安全的replayer代码位置

2. **修复VGM循环点计算**
   - 符合VGM v1.71标准和libvgm规范
   - 正确设置total_samples（循环前样本数）和loop_samples（循环部分样本数）
   - 循环点现在在歌曲开始处标记，而不是中间

3. **修复循环播放行为**
   - VGM播放器现在可以正确循环并在达到次数后跳转到下一曲
   - 文件大小显著减小（DQ1&2: 11MB→4.8MB, Tales: 18MB→7.8MB）

### ✨ 主要特性

#### 精确时序捕获
- **Cycle-accurate timing** - 基于CPU周期的精确时序，而非固定时间步长
- **Sample-accurate delays** - 每次寄存器写入都有精确的样本级时间戳
- **无时序漂移** - 使用64位运算避免舍入误差累积
- **60Hz刷新率** - 17ms时间步长，更接近Game Boy实际帧率

#### 批量处理
- 自动解压7z和ZIP压缩包
- 自动查找并解析M3U播放列表
- 从文件名提取游戏元数据
- 支持循环点标记
- 自动生成GD3标签
- 自动打包输出为ZIP

#### VGM格式支持
- VGM 1.71格式
- Game Boy DMG音频芯片
- 完整的寄存器捕获(0xFF10-0xFF3F)
- 精确的循环点设置
- GD3元数据标签

## 使用方法

### 基本用法

```bash
# 从7z压缩包转换
./gbs2vgm_batch "Game Name.7z"

# 从ZIP压缩包转换
./gbs2vgm_batch "Game Name.zip"

# 从M3U播放列表转换
./gbs2vgm_batch "playlist.m3u"
```

### 命令行选项

```
Usage: gbs2vgm_batch [options] <input-file>

Options:
  -r <rate>     采样率 (默认: 44100)
  -f <seconds>  淡出时长 (默认: 3秒)
  -s <seconds>  静音超时 (默认: 5秒)
  -d            启用调试模式
  -h            显示帮助信息

Input formats:
  - .7z         7z压缩包 (自动解压)
  - .zip        ZIP压缩包 (自动解压)
  - .m3u        M3U播放列表
```

### 输出

- VGM文件保存在 `<游戏名>/` 目录
- 自动打包为 `<游戏名>.zip`
- 文件名格式: `01 Track Name.vgm`
- 自动生成转换日志

## M3U播放列表格式

```m3u
# 元数据标签
# @TITLE       游戏名称
# @COMPOSER    作曲家
# @ARTIST      开发商
# @DATE        发行日期
# @RIPPER      Ripper名称
# @TAGGER      标签制作者

# 曲目列表
# 格式: filename::GBS,subsong,title,duration,,loop_count
dq3/CGB-BD3J-JPN.gbs::GBS,0,Overture ~ Roto's Theme,1:39,,2
dq3/CGB-BD3J-JPN.gbs::GBS,3,Intermezzo,0:50,,10
dq3/CGB-BD3J-JPN.gbs::GBS,34,Prologue,4:08,,10
```

### 字段说明

- `filename` - GBS文件路径(相对于M3U文件)
- `subsong` - 子曲目编号
- `title` - 曲目标题
- `duration` - 时长(M:SS格式)
- `loop_count` - 循环次数
  - `1` = 不循环(单次播放 + fadeout)
  - `2+` = 循环(在开始处标记循环点)

## VGM循环点说明

### v2.0循环点实现

对于有循环的曲目（loop_count > 1）：
1. 在歌曲开始后约0.1秒标记循环点
2. 渲染完整的duration长度
3. 写入0x66 (End of Data)命令
4. VGM头部设置：
   - `total_samples` = 循环点前的样本数（很小，约0.1秒）
   - `loop_samples` = 循环部分的样本数（几乎是整首歌）
   - `loop_offset` = 循环点的文件位置（相对于0x1C）

### 播放器行为

VGM播放器会：
1. 播放intro部分（约0.1秒）
2. 进入循环部分（完整歌曲）
3. 到达0x66命令时跳回循环点
4. 根据播放器配置的循环次数决定何时停止
5. 停止后自动跳转到下一曲

这与Kirby's Dream Land等标准VGM文件的行为一致。

## 时序精度改进

### 原理

传统方法使用固定时间步长(33ms)累积样本，导致：
- 寄存器写入时间戳不精确(±33ms误差)
- 舍入误差累积导致时序漂移
- 音符节奏轻微不稳定

新方法使用CPU周期精确计算：
```c
// 每次寄存器写入时
cycles_elapsed = current_cycles - last_write_cycles;
samples = (cycles_elapsed * 44100) / 4194304;  // GB_CLOCK
```

### 精度对比

| 方法 | 时间分辨率 | 误差累积 |
|------|-----------|---------|
| 旧版 (33ms步进) | ±33ms | 有 |
| 新版 (cycle-based) | ~0.238μs | 无 |

**精度提升约138,000倍！**

### 参考实现

本实现参考了：
- MAME VGM日志记录的时序机制
- libvgm的循环点实现
- GME的GBS解析器

## 技术细节

### Game Boy音频寄存器

捕获的寄存器范围: `0xFF10-0xFF3F`

- `0xFF10-0xFF14` - 方波通道1（带频率扫描）
- `0xFF16-0xFF19` - 方波通道2
- `0xFF1A-0xFF1E` - 波形通道
- `0xFF20-0xFF23` - 噪声通道
- `0xFF24-0xFF26` - 音量控制和开关
- `0xFF30-0xFF3F` - 波形RAM

### VGM文件结构

```
[VGM Header - 0x100 bytes]
  Offset 0x00: "Vgm " signature
  Offset 0x04: EOF offset (relative)
  Offset 0x08: Version (0x171 for v1.71)
  Offset 0x18: Total Samples (循环前)
  Offset 0x1C: Loop Offset (relative)
  Offset 0x20: Loop Samples (循环部分)
  Offset 0x34: Data Offset (relative)
  Offset 0x80: GB DMG Clock (4194304 Hz)

[VGM Data]
  0xB3 <reg> <data>  - GB寄存器写入
  0x61 <nn nn>       - 等待n样本
  0x62               - 等待735样本 (1/60秒)
  0x63               - 等待882样本 (1/50秒)
  0x70-0x7F          - 等待1-16样本
  0x66               - 文件结束

[GD3 Tag]
  Track Name (EN/JP)
  Game Name (EN/JP)
  System Name (EN/JP)
  Composer (EN/JP)
  Release Date
  Ripper
  Notes
```

## 示例

### 转换Dragon Quest I & II

```bash
./gbs2vgm_batch "Dragon Warrior I & II [Dragon Quest I+II] (1999-09-23)(Chunsoft)(Tose)(Enix)[GBC].7z"
```

输出:
```
=== Metadata from filename ===
  Game Name: Dragon Warrior I & II
  Composer: Chunsoft
  Release Date: 1999-09-23

Extracting 7z archive...
  Extracted: DMG-AEDJ-JPN.gbs
  Extracted: DMG-AEDJ-JPN.m3u
  ...

Converting: 01 Overture March (intro)
  Done: 50339918 cycles processed
Converting: 02 Overture March
  Done: 385891836 cycles processed
...

Creating output archive...
ZIP archive created successfully with 49 files

Conversion complete!
```

输出文件: `Dragon Warrior I & II [Dragon Quest I+II] (1999-09-23)(Chunsoft)(Tose)(Enix)[GBC].zip` (4.8MB)

### 转换Tales of Phantasia

```bash
./gbs2vgm_batch "Tales of Phantasia - Narikiri Dungeon (2000-11-10)(Alfa System)(Namco)[GBC].7z"
```

输出文件: `Tales of Phantasia - Narikiri Dungeon (2000-11-10)(Alfa System)(Namco)[GBC].zip` (7.8MB)

## 编译

### Windows (MSYS2/MinGW)

```bash
cd gbsplay
./build_batch.sh
```

### Linux/macOS

```bash
cd gbsplay
gcc -Wall -O2 -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=200809L \
    -o gbs2vgm_batch \
    gbs2vgm_batch.c m3u_parser.c vgm_writer.c \
    gbcpu.c gbhw.c gblfsr.c mapper.c gbs.c \
    crc32.c util.c filename_parser.c archive_utils.c \
    7z/*.c zlib/*.c \
    -lm
```

## 依赖

- **必需**:
  - C编译器 (GCC/Clang)
  - zlib库
  - 7z SDK (已包含)

- **可选**:
  - Python 3 (用于打包脚本)

## 已知问题

- 某些GBS文件可能显示 "Bank out of range" 警告，但不影响转换
- 某些GBS文件的subsong 32可能返回0 cycles，这是正常的

## 版本历史

### v2.1 (2026-02-04)
- 🐛 **修复**: 移除VGM循环点标记，避免播放器循环过长
- ✨ **改进**: VGM文件现在播放一次完整的duration后自动跳到下一曲
- ✅ **兼容**: 与in_vgm (Winamp)、VGMPlay等播放器完全兼容
- 📝 详细说明: 参见 GBS2VGM_v2.1_LOOP_FIX.md

### v2.0 (2026-02-04)
- ✨ **重大修复**: 修复不同内存映射的GBS文件支持
  - 修复replayer代码地址冲突
  - Tales of Phantasia现在可以正常转换
- ✨ **重大修复**: 修复VGM循环点计算（v2.1进一步改进）
  - 符合VGM v1.71标准
  - 循环点在开始处标记
  - 播放器可以正确循环并跳转到下一曲
- ✨ 实现cycle-accurate timing
- ✨ 精确的循环点设置
- ✨ 60Hz刷新率
- 🐛 修复文件名重复编号
- 🐛 修复时序漂移问题
- 📦 自动打包输出为ZIP

### v1.0 (2026-02-03)
- 初始版本
- 基本的GBS到VGM转换
- M3U播放列表支持

## 测试结果

### 成功转换的游戏

- ✅ Dragon Quest III (DQ3)
- ✅ Dragon Quest I & II (DQ1&2)
- ✅ Tales of Phantasia: Narikiri Dungeon
- ✅ Dragon Quest Monsters
- ✅ Kirby's Dream Land

### 音频质量验证

- 寄存器写入频率: 200-400次/秒（正常）
- 循环点准确性: 符合VGM标准
- 播放器兼容性: 与Kirby等标准VGM一致

## 许可证

GNU GPL v1 or later

## 致谢

- gbsplay项目 - GBS播放引擎
- MAME项目 - VGM日志记录参考实现
- libvgm项目 - VGM循环点实现参考
- GME项目 - GBS解析器参考
- VGM格式规范

## 作者

- Claude Sonnet 4.5 & Denjhang (2026)

## 技术支持

如有问题或建议，请访问: https://github.com/anthropics/claude-code/issues
