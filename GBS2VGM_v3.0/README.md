# GBS2VGM v3.0 - Game Boy Sound to VGM Converter

**完整的GBS到VGM转换工具，支持智能循环点检测和自动修剪**

[![Version](https://img.shields.io/badge/version-3.0-blue.svg)](https://github.com/yourusername/gbs2vgm)
[![License](https://img.shields.io/badge/license-GPL%20v1-green.svg)](LICENSE)

---

## 🎵 简介

GBS2VGM是一个强大的工具，可以将Game Boy Sound (GBS)文件转换为VGM (Video Game Music)格式，并自动检测和设置循环点。

### 主要特性

- ✅ **GBS到VGM转换** - 完整支持Game Boy音频芯片
- ✅ **智能循环点检测** - 自动识别最佳循环位置
- ✅ **自动修剪** - 减少文件大小，保留完整音质
- ✅ **M3U播放列表支持** - 批量转换多个曲目
- ✅ **一键完整流程** - 从GBS到最终VGM，全自动化
- ✅ **循环点自动设置** - VGM文件可在播放器中正确循环

---

## 📦 发布内容

```
GBS2VGM_v3.0/
├── bin/                          # 可执行文件和脚本
│   ├── gbs2vgm_batch.exe        # 主程序
│   ├── vgm_trim.exe             # VGM修剪工具
│   ├── vgm_parser.exe           # VGM信息查看器
│   ├── test_m3u_parser.exe      # M3U解析测试
│   ├── gbs2vgm_final.sh         # 完整自动化流程 (推荐)
│   ├── auto_trim_smart.sh       # 智能修剪脚本
│   ├── merge_trimmed.sh         # 文件合并脚本
│   └── cleanup_vgm.sh           # 清理工具
│
├── src/                          # 源代码
│   ├── Makefile                 # 编译配置
│   ├── vgm_trim.c               # VGM修剪工具源码
│   └── gbsplay/                 # GBS播放引擎源码
│       ├── gbs2vgm_batch.c      # 主程序源码
│       ├── m3u_parser.c         # M3U解析器
│       ├── vgm_writer.c         # VGM写入器
│       ├── vgmlpfnd.c           # 循环点检测
│       ├── vgm_trml.c           # VGM修剪核心
│       └── ...                  # 其他源文件
│
├── docs/                         # 文档
│   ├── README_v2.2.md           # 技术文档
│   ├── 使用说明_v2.2.md         # 使用说明
│   ├── 完整使用指南.md          # 详细指南
│   ├── 完成报告_v2.3.md         # 开发报告
│   └── 项目总结.md              # 项目总结
│
└── examples/                     # 示例文件
    ├── test_intro_loop.m3u      # M3U示例
    └── test_simple.m3u          # 简单示例
```

---

## 🚀 快速开始

### Windows用户 (推荐)

1. **下载并解压** `GBS2VGM_v3.0.zip`

2. **运行转换** (在MSYS2或Git Bash中):
```bash
cd GBS2VGM_v3.0/bin
./gbs2vgm_final.sh "path/to/your.m3u" "output_folder"
```

3. **播放VGM文件**:
   - 使用VGMPlay、foobar2000或其他VGM播放器

### 从源码编译

需要MSYS2环境和以下依赖：
- gcc
- make
- zlib

```bash
cd GBS2VGM_v3.0/src
make
make utils
```

---

## 📖 使用指南

### 方法1: 一键完整流程 (推荐)

```bash
./gbs2vgm_final.sh "input.m3u" "output_dir"
```

这个脚本会自动完成：
1. GBS → VGM 转换
2. 循环点检测
3. 智能修剪
4. 循环点设置

### 方法2: 分步执行

**步骤1: 转换GBS到VGM**
```bash
./gbs2vgm_batch.exe "input.m3u" "output_dir"
```

**步骤2: 智能修剪**
```bash
cd output_dir
../auto_trim_smart.sh
```

**步骤3: 合并文件**
```bash
../merge_trimmed.sh
```

### 方法3: 只修剪现有VGM文件

```bash
./auto_trim_smart.sh
```

---

## 🎯 功能详解

### 智能循环点检测

GBS2VGM使用先进的算法自动检测循环点：

1. **优先级1**: 使用vgmlpfnd推荐的最佳循环点（带"!"标记）
2. **优先级2**: 选择持续时间最长的循环点（避免短片段）
3. **过滤**: 自动过滤命令数<1000的短循环

### VGM自动修剪

- **减少文件大小**: 平均减少40-80%
- **保持音质**: 只包含intro + 一次完整循环
- **设置循环点**: 自动设置VGM头部循环偏移
- **兼容性**: 所有VGM播放器都能正确循环

### M3U格式支持

支持两种M3U格式：

**旧格式** (5个逗号):
```
filename.gbs::GBS,subsong,title,duration,,loop_count
```

**新格式** (6个逗号, GME兼容):
```
filename.gbs::GBS,subsong,title,duration,intro-loop,fade,loop_count
```

---

## 📊 转换效果

### Dragon Quest I & II 测试结果

- **总曲目**: 49首
- **修剪文件**: 11首
- **文件大小**: 13MB → 9.1MB (减少30%)
- **平均减少**: 40-80% (修剪的文件)

### 修剪示例

| 曲目 | 原始 | 修剪后 | 减少 |
|------|------|--------|------|
| Ladutorm Castle | 822KB | 165KB | 81% |
| People of the Town | 689KB | 117KB | 84% |
| Fight | 1.1MB | 584KB | 47% |
| King Dragon | 679KB | 349KB | 49% |

---

## 🛠️ 工具说明

### gbs2vgm_batch.exe
主转换程序，将GBS文件转换为VGM格式。

**用法**:
```bash
./gbs2vgm_batch.exe <m3u文件> <输出目录>
```

### vgm_trim.exe
VGM修剪工具，根据循环点修剪VGM文件。

**用法**:
```bash
./vgm_trim.exe <输入.vgm> <起始样本> <循环起始> <循环结束> <输出.vgm>
```

### gbs2vgm_final.sh
完整自动化流程脚本（推荐使用）。

**用法**:
```bash
./gbs2vgm_final.sh <m3u文件> <输出目录>
```

### auto_trim_smart.sh
智能修剪脚本，自动选择最佳循环点。

**用法**:
```bash
./auto_trim_smart.sh
```

### vgm_parser.exe
VGM文件信息查看器。

**用法**:
```bash
./vgm_parser.exe <文件.vgm>
```

### cleanup_vgm.sh
清理VGM文件夹中的临时文件。

**用法**:
```bash
./cleanup_vgm.sh [目录]
```

---

## 🔧 技术细节

### 支持的音频芯片

- **Game Boy DMG** (4194304 Hz)
  - 方波1 (扫频)
  - 方波2
  - 波形通道
  - 噪声通道

### VGM版本

- **输出版本**: VGM 1.71
- **采样率**: 44100 Hz
- **循环支持**: 完整支持

### 循环检测原理

通过比较3次播放的命令序列，找到重复模式：

```
播放1: [intro] [loop] [loop] [loop]
播放2:         [loop] [loop] [loop]
播放3:         [loop] [loop] [loop]
           ↑        ↑
      循环开始  循环结束
```

---

## 🐛 故障排除

### 问题1: 编译失败

**解决**:
```bash
# 确保在MSYS2环境中
pacman -S gcc make zlib-devel

# 清理并重新编译
make clean
make
make utils
```

### 问题2: 转换失败 - 文件很小

**原因**: 可能是旧版本的整数溢出bug

**解决**: 确保使用v3.0或更高版本

### 问题3: 未找到循环点

**原因**:
- 循环模式太复杂
- 录制长度不足
- 曲目本身不循环

**解决**:
- 检查M3U文件中的loop_count设置
- 手动调整vgmlpfnd参数

### 问题4: Bank警告

**现象**: "Bank 112 out of range (0-8)!"

**影响**: 无，可以忽略

**原因**: GBS文件访问超出范围的ROM bank（正常现象）

---

## 📝 更新日志

### v3.0 (2026-02-05)
- ✨ 发布完整版本
- ✨ 整理所有源码和依赖
- ✨ 创建标准发布包
- 📚 完善所有文档
- 🎯 准备公开发布

### v2.3 (2026-02-05)
- ✨ 智能循环点选择算法
- ✨ VGM自动修剪功能
- ✨ 循环点自动设置
- ✨ 一键完整流程
- 🐛 修复循环点检测问题

### v2.2 (2026-02-04)
- 🐛 修复整数溢出bug
- ✨ 支持M3U旧格式
- ✨ 集成vgmlpfnd
- ✨ 创建Makefile

---

## 🙏 致谢

### 开源项目
- **[gbsplay](https://github.com/mmitch/gbsplay)** - GBS播放引擎
- **[vgmtools](https://github.com/vgmrips/vgmtools)** - VGM工具集
- **[Game Music Emu](https://bitbucket.org/mpyne/game-music-emu/)** - M3U格式参考
- **[LZMA SDK](https://www.7-zip.org/sdk.html)** - 7z解压支持

### 贡献者
- **Denjhang** - 项目发起人、测试、反馈
- **Claude Sonnet 4.5** - AI编程助手

---

## 📄 许可证

GNU GPL v1 or later

本项目基于以下开源项目：
- gbsplay (GPL)
- vgmtools (GPL)
- LZMA SDK (Public Domain)

---

## 📞 联系方式

### 问题反馈
- GitHub Issues: [创建Issue](https://github.com/yourusername/gbs2vgm/issues)
- Email: your.email@example.com

### 贡献代码
欢迎提交Pull Request：
1. Fork项目
2. 创建特性分支
3. 提交代码
4. 发起Pull Request

---

## 🌟 相关链接

- **VGM文件格式**: http://vgmrips.net/wiki/VGM_Specification
- **GBS文件格式**: https://gbdev.gg8.se/wiki/articles/Gameboy_sound_hardware
- **VGMPlay**: https://github.com/vgmrips/vgmplay
- **foobar2000 VGM插件**: https://www.foobar2000.org/components

---

## 🎊 结语

GBS2VGM v3.0提供了完整的GBS到VGM转换解决方案，从转换、检测到修剪，全程自动化。

**感谢使用GBS2VGM！** 🎵

如有问题或建议，欢迎反馈！

---

**版本**: v3.0
**发布日期**: 2026-02-05
**维护者**: Claude Sonnet 4.5 & Denjhang
