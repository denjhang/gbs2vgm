# GBS2VGM - Game Boy Sound to VGM Converter

## 简介

GBS2VGM 是一个强大的 Game Boy 音乐转换工具，可以将 GBS (Game Boy Sound) 文件转换为 VGM (Video Game Music) 格式。

### 主要特性

- ✅ **批量转换** - 支持 M3U 播放列表批量转换
- ✅ **7z/ZIP 支持** - 自动解压 7z 和 ZIP 压缩包
- ✅ **智能元数据** - 从文件名和 GBS 文件自动提取元数据
- ✅ **完整 GD3 标签** - 包含游戏名、作者、发行日期等信息
- ✅ **ZIP 打包输出** - 自动将转换结果打包为 ZIP
- ✅ **无外部依赖** - 集成 7z SDK 和 zlib，无需安装额外软件

## 文件说明

### 可执行文件
- `gbs2vgm.exe` - 单文件转换器
- `gbs2vgm_batch.exe` - 批量转换器（支持 7z/ZIP）

### 工具程序
- `vgm_parser.exe` - VGM 文件分析器
- `gd3_reader.exe` - GD3 标签读取器
- `test_filename_parser.exe` - 文件名解析测试工具

### 源代码
- `src/` - 所有源代码文件
  - `gbs2vgm.c` - 单文件转换器源码
  - `gbs2vgm_batch.c` - 批量转换器源码
  - `vgm_writer.c/h` - VGM 写入器
  - `m3u_parser.c/h` - M3U 解析器
  - `filename_parser.c/h` - 文件名解析器
  - `archive_utils.c/h` - 压缩包处理
  - `7z/` - 7z SDK 源码
  - `zlib/` - zlib 库源码

### 文档
- `README.md` - 本文件
- `README_ARCHIVE.md` - 压缩包支持说明
- `CHANGELOG.md` - 更新日志

### 演示文件
- `examples/` - 示例文件
  - `dq3_complete.m3u` - Dragon Warrior III 完整播放列表示例

## 使用方法

### 基本用法

#### 1. 转换单个 GBS 文件
```bash
gbs2vgm.exe input.gbs output.vgm
```

#### 2. 转换 M3U 播放列表
```bash
gbs2vgm_batch.exe playlist.m3u
```

#### 3. 转换 7z 压缩包
```bash
gbs2vgm_batch.exe "Dragon Warrior III.7z"
```

#### 4. 指定输出目录
```bash
gbs2vgm_batch.exe game.7z output_folder
```

### 文件名格式

批量转换器支持从文件名自动提取元数据，推荐的文件名格式：

```
Game Name [Alt Name] (YYYY-MM-DD)(Developer)(Publisher)(Company)[System].7z
```

示例：
```
Dragon Warrior III [Dragon Quest III - Soshite Densetsu he...] (2000-12-08)(Chunsoft)(Tose)(Enix)[GBC].7z
```

解析结果：
- 游戏名：Dragon Warrior III
- 别名：Dragon Quest III - Soshite Densetsu he...
- 发行日期：2000-12-08
- 开发商：Chunsoft
- 发行商：Tose
- 公司：Enix
- 系统：GBC

### M3U 格式

M3U 文件支持以下元数据标签：

```m3u
# @TITLE       游戏名称
# @COMPOSER    作曲家
# @ARTIST      艺术家/公司
# @DATE        发行日期
# @RIPPER      Ripper 名称

filename.gbs::GBS,subsong,Track Title,duration,,loop_count
```

示例：
```m3u
# @TITLE       Dragon Warrior III
# @COMPOSER    Koichi Sugiyama
# @DATE        2000-12-08
# @RIPPER      Denjhang

CGB-BD3J-JPN.gbs::GBS,0,Overture ~ Roto's Theme,1:39,,2
CGB-BD3J-JPN.gbs::GBS,3,Intermezzo,0:50,,10
```

## 输出说明

### 单文件转换
- 输出：单个 VGM 文件

### M3U 转换
- 输出：`[游戏名]_vgm/` 目录，包含所有 VGM 文件

### 7z/ZIP 转换
- 输出：`[游戏名]_vgm.zip` 压缩包，包含所有 VGM 文件

## GD3 标签

转换后的 VGM 文件包含完整的 GD3 标签：

- **Track Name** - 曲目名称（从 M3U 获取）
- **Game Name** - 游戏名称（从文件名或 M3U 获取）
- **System Name** - 系统名称（固定为 "Game Boy"）
- **Author Name** - 作者名称（从 GBS 文件内部读取）
- **Release Date** - 发行日期（从文件名或 M3U 获取）
- **VGM Creator** - 转换者（从 M3U 的 @RIPPER 获取，默认 "Denjhang"）
- **Notes** - 备注（固定为 "gbs2vgm by Claude & Denjhang"）

## 编译说明

### Windows (MSYS2/MinGW)

```bash
cd src
./build_batch.sh
```

或手动编译：

```bash
# 编译批量转换器
gcc -c gbs2vgm_batch.c -o gbs2vgm_batch.o
gcc -c filename_parser.c -o filename_parser.o
gcc -c archive_utils.c -I7z -Izlib -o archive_utils.o
gcc -c vgm_writer.c -o vgm_writer.o
gcc -c m3u_parser.c -o m3u_parser.o

# 编译 7z SDK
gcc -c -O2 7z/*.c

# 编译 zlib
gcc -c -O2 zlib/*.c

# 链接
gcc -o gbs2vgm_batch.exe \
    gbs2vgm_batch.o filename_parser.o archive_utils.o \
    vgm_writer.o m3u_parser.o \
    gbcpu.o gbhw.o gblfsr.o mapper.o gbs.o crc32.o util.o \
    7z/*.o zlib/*.o -lm
```

## 技术细节

### VGM 格式
- 版本：1.71
- Game Boy DMG 时钟：4194304 Hz (0x00400000)
- 时钟偏移：0x80（符合 VGM 1.61+ 规范）

### 支持的压缩格式
- **7z** - LZMA、LZMA2、PPMd、BCJ2 等
- **ZIP** - Deflate 压缩

### 依赖库
- **7z SDK** - 用于 7z 解压（已集成）
- **zlib** - 用于 ZIP 解压和打包（已集成）
- **gbsplay** - Game Boy 音频模拟器核心

## 常见问题

### Q: 为什么转换后的 VGM 文件没有声音？
A: 这个问题已在最新版本中修复。确保使用的是修正后的版本，Game Boy 时钟已正确设置在 0x80 偏移位置。

### Q: 如何批量转换多个 7z 文件？
A: 可以使用批处理脚本：
```bash
for file in *.7z; do
    gbs2vgm_batch.exe "$file"
done
```

### Q: 作者名显示不正确？
A: 最新版本会从 GBS 文件内部读取作者信息，而不是使用文件名中的公司名。

### Q: 支持哪些 Game Boy 系统？
A: 支持 Game Boy (DMG)、Game Boy Color (GBC) 和 Game Boy Advance (GBA) 的 GBS 文件。

## 更新日志

### v1.0.0 (2026-02-03)
- ✅ 初始发布
- ✅ 支持 M3U 播放列表批量转换
- ✅ 支持 7z/ZIP 自动解压
- ✅ 智能文件名解析
- ✅ 完整 GD3 标签支持
- ✅ 从 GBS 文件读取作者信息
- ✅ 修正 Game Boy 时钟偏移（0x7C → 0x80）
- ✅ 智能 M3U 文件查找（优先匹配 GBS 文件名）
- ✅ ZIP 自动打包输出

## 致谢

- **gbsplay** - Game Boy 音频模拟器核心
- **7z SDK** - Igor Pavlov
- **zlib** - Jean-loup Gailly 和 Mark Adler
- **nsf2vgm** - 压缩包处理参考实现
- **libvgm** - VGM 格式参考

## 许可证

本项目基于 GNU GPL v1 或更高版本授权。

## 作者

- **Claude & Denjhang** - 2026

## 联系方式

如有问题或建议，请通过 GitHub Issues 反馈。

---

**Enjoy converting Game Boy music! 🎵**
