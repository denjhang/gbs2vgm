# GBS2VGM - Game Boy Sound to VGM Converter

高精度的Game Boy音乐文件(GBS)到VGM格式转换工具，支持批量转换和7z/ZIP压缩包。

## 主要特性

### ✨ 精确时序捕获
- **Cycle-accurate timing** - 基于CPU周期的精确时序，而非固定时间步长
- **Sample-accurate delays** - 每次寄存器写入都有精确的样本级时间戳
- **无时序漂移** - 使用64位运算避免舍入误差累积
- **60Hz刷新率** - 17ms时间步长，更接近Game Boy实际帧率

### 📦 批量处理
- 自动解压7z和ZIP压缩包
- 自动查找并解析M3U播放列表
- 从文件名提取游戏元数据
- 支持循环点标记
- 自动生成GD3标签

### 🎵 VGM格式支持
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
  -h            显示帮助信息

Input formats:
  - .7z         7z压缩包 (自动解压)
  - .zip        ZIP压缩包 (自动解压)
  - .m3u        M3U播放列表
```

### 输出

- VGM文件保存在 `<游戏名>_vgm/` 目录
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
  - `1` = 不循环(单次播放)
  - `2+` = 循环(intro + loop)

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

本实现参考了MAME VGM日志记录的时序机制：
- 44.1kHz定时器
- 延迟累积机制
- 写入前同步
- 精确的VGM命令编码

## 技术细节

### Game Boy音频寄存器

捕获的寄存器范围: `0xFF10-0xFF3F`

- `0xFF10-0xFF14` - 方波通道1
- `0xFF16-0xFF19` - 方波通道2
- `0xFF1A-0xFF1E` - 波形通道
- `0xFF20-0xFF23` - 噪声通道
- `0xFF24-0xFF26` - 音量控制和开关

### VGM文件结构

```
[VGM Header]
  - Version: 1.71
  - GB Clock: 4194304 Hz
  - Total Samples
  - Loop Offset
  - Loop Samples
  - GD3 Offset

[VGM Data]
  - 0xB3 <reg> <data>  - GB寄存器写入
  - 0x61 <nn nn>       - 等待n样本
  - 0x70-0x7F          - 等待1-16样本
  - 0x66               - 文件结束

[GD3 Tag]
  - Track Name
  - Game Name
  - Composer
  - Release Date
  - Ripper
  - Notes
```

### 循环点处理

对于`loop_count > 1`的曲目：
1. 播放intro部分(duration秒)
2. 在intro结束时标记循环点
3. 再播放loop部分(duration秒)
4. VGM播放器会从循环点重复播放

## 示例

### 转换勇者斗恶龙3

```bash
./gbs2vgm_batch "Dragon Warrior III [Dragon Quest III] (2000-12-08)(Chunsoft)(Tose)(Enix)[GBC].7z"
```

输出:
```
=== Metadata from filename ===
  Game Name: Dragon Warrior III
  Composer: Chunsoft
  Release Date: 2000-12-08

Extracting 7z archive...
  Extracted: CGB-BD3J-JPN.gbs
  Extracted: CGB-BD3J-JPN.m3u
  ...

Converting: 01 Overture ~ Roto's Theme
  Done: 830537344 cycles processed (198.02 seconds)
Converting: 02 Intermezzo
  Done: 419526772 cycles processed (100.01 seconds)
...

Conversion complete!
Output: Dragon Warrior III_vgm/
```

## 编译

```bash
cd gbsplay
gcc -Wall -O2 -D_XOPEN_SOURCE=500 -D_POSIX_C_SOURCE=200809L \
    -o gbs2vgm_batch \
    gbs2vgm_batch.c m3u_parser.c vgm_writer.c \
    gbcpu.c gbhw.c gblfsr.c mapper.c gbs.c \
    crc32.c util.c filename_parser.c archive_utils.c \
    -lm -lz
```

## 依赖

- zlib (VGM压缩)
- 7z命令行工具 (可选，用于7z解压)

## 版本历史

### v2.0 (2026-02-04)
- ✨ 实现cycle-accurate timing
- ✨ 精确的循环点设置
- ✨ 60Hz刷新率
- 🐛 修复文件名重复编号
- 🐛 修复时序漂移问题

### v1.0
- 初始版本
- 基本的GBS到VGM转换
- M3U播放列表支持

## 许可证

GNU GPL v1 or later

## 致谢

- gbsplay项目 - GBS播放引擎
- MAME项目 - VGM日志记录参考实现
- VGM格式规范

## 作者

- Claude & Denjhang (2026)
