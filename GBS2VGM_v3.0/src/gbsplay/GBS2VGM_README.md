# gbs2vgm - GBS转VGM转换器（支持M3U）

将Game Boy Sound (GBS)文件转换为VGM格式的控制台程序，支持使用M3U播放列表文件获取曲目信息。

## 功能特性

- 读取M3U播放列表文件及曲目元数据
- 将GBS子歌曲转换为独立的VGM文件
- 根据M3U中的曲目标题自动命名输出文件
- 支持自定义采样率和淡出时长
- 捕获Game Boy硬件寄存器写入并转换为VGM命令
- 生成标准VGM 1.71格式文件
- **支持GD3标签**（曲目名称、游戏名称、作者、发行日期等）
- **支持循环点标记**（自动检测M3U中的循环次数）

## 编译

```bash
cd gbsplay
make -f Makefile.gbs2vgm
```

这将生成 `gbs2vgm` 可执行文件。

## 使用方法

```bash
./gbs2vgm [选项] <m3u文件>
```

### 选项

- `-r <采样率>` - 采样率（默认：44100 Hz）
- `-f <秒数>` - 淡出时长（默认：3秒）
- `-o <目录>` - 输出目录（默认：当前目录）
- `-h` - 显示帮助信息

### 示例

转换M3U文件中的所有曲目：
```bash
./gbs2vgm ../dq3/CGB-BD3J-JPN.m3u
```

使用自定义输出目录：
```bash
./gbs2vgm -o output_vgm ../dq3/CGB-BD3J-JPN.m3u
```

自定义采样率和淡出：
```bash
./gbs2vgm -r 48000 -f 5 -o output_vgm ../dq3/CGB-BD3J-JPN.m3u
```

## M3U格式

M3U文件应包含以下格式的曲目信息：

```
# @TITLE       游戏标题
# @ARTIST      艺术家名称
# @COMPOSER    作曲家名称
# @DATE        发行日期

文件名.gbs::GBS,子歌曲编号,曲目标题,时长,,循环次数
```

示例：
```
# @TITLE       Dragon Warrior III
# @ARTIST      Chunsoft, Tose, Enix
# @COMPOSER    Koichi Sugiyama
# @DATE        2000-12-08

CGB-BD3J-JPN.gbs::GBS,0,Overture ~ Roto's Theme,1:39,,1
CGB-BD3J-JPN.gbs::GBS,10,Town,1:04,,10
```

**循环次数说明**：
- `1` = 不循环，只播放一次（添加淡出）
- `>1` = 循环播放，VGM文件会包含intro+一次完整循环，并标记循环点

## 输出

程序创建的VGM文件命名格式：
```
<曲目编号> <曲目标题>.vgm
```

例如：
- `01 Overture ~ Roto's Theme.vgm` (80KB, 不循环)
- `02 Town.vgm` (122KB, 包含循环点)
- `03 Prologue.vgm` (324KB, 包含循环点)

## VGM格式说明

生成的VGM文件：
- **版本**：1.71
- **芯片**：Game Boy DMG (时钟频率 4194304 Hz)
- **GD3标签**：包含曲目名称、游戏名称、系统名称、作者、发行日期
- **循环支持**：自动标记循环点和循环采样数
- **命令**：
  - `0xB3 aa dd` - Game Boy寄存器写入
  - `0x61 nn nn` - 等待n个采样
  - `0x70-0x7F` - 短等待（1-16个采样）
  - `0x66` - 数据结束

## 技术细节

- 使用gbsplay库进行GBS播放
- 通过IO回调捕获Game Boy硬件寄存器写入（0xFF10-0xFF3F）
- 基于时间步长（33ms）计算等待命令
- 支持各种内存布局的GBS文件
- 播放器代码放置在地址0x3000以避免与GBS数据冲突
- GD3标签使用UTF-16LE编码
- 循环点在第一次循环结束时自动标记

## 测试

dq3文件夹中提供了测试文件：
```bash
./gbs2vgm -o test_vgm ../dq3/CGB-BD3J-JPN.m3u
```

这将把《勇者斗恶龙III》的所有64首曲目转换为VGM文件。

## 与gbs2wav的区别

| 特性 | gbs2wav | gbs2vgm |
|------|---------|---------|
| 输出格式 | WAV (PCM音频) | VGM (日志文件) |
| 文件大小 | 大（6-31MB） | 小（80-324KB） |
| 播放方式 | 直接播放 | 需要VGM播放器 |
| 音质 | 渲染后的音频 | 原始芯片命令 |
| 循环支持 | 无 | 有（标记循环点） |
| 元数据 | 无 | 有（GD3标签） |
| 用途 | 音频播放 | 芯片音乐保存/分析 |

VGM文件的优势：
- 文件体积小（通常是WAV的1/100）
- 保留原始芯片音色
- 可以在VGM播放器中播放（支持循环）
- 包含完整的元数据（GD3标签）
- 可以转换为其他格式

## 许可证

根据GNU GPL v1或更高版本授权。
