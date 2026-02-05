# GBS2VGM v3.0 - Game Boy音乐转VGM工具

**完整的GBS到VGM转换工具，支持智能循环点检测和自动修剪**

---

## 🎵 简介

GBS2VGM是一个强大的工具，可以将Game Boy Sound (GBS)文件转换为VGM (Video Game Music)格式，并自动检测和设置循环点。

### 主要特性

- ✅ **GBS到VGM转换** - 完整支持Game Boy音频芯片
- ✅ **智能循环点检测** - 自动识别最佳循环位置
- ✅ **自动修剪** - 减少文件大小40-80%，保留完整音质
- ✅ **M3U播放列表支持** - 批量转换多个曲目
- ✅ **一键完整流程** - 从GBS到最终VGM，全自动化
- ✅ **循环点自动设置** - VGM文件可在播放器中正确循环

---

## 🚀 快速开始

### Windows用户 (推荐)

1. **下载并解压** `GBS2VGM_v3.0.zip`

2. **运行转换** (在MSYS2或Git Bash中):
```bash
cd GBS2VGM_v3.0/bin
./gbs2vgm_final.sh "你的文件.m3u" "输出文件夹"
```

3. **播放VGM文件**:
   - 使用VGMPlay、foobar2000或其他VGM播放器

### 从源码编译

需要MSYS2环境：
```bash
cd GBS2VGM_v3.0/src
make
make utils
```

---

## 📖 使用方法

### 方法1: 一键完整流程 (推荐)

```bash
./gbs2vgm_final.sh "输入.m3u" "输出目录"
```

自动完成：转换 → 循环检测 → 智能修剪 → 循环点设置

### 方法2: 分步执行

**步骤1: 转换**
```bash
./gbs2vgm_batch.exe "输入.m3u" "输出目录"
```

**步骤2: 智能修剪**
```bash
cd 输出目录
../auto_trim_smart.sh
```

**步骤3: 合并文件**
```bash
../merge_trimmed.sh
```

---

## 🎯 功能详解

### 智能循环点检测

- **优先级1**: 使用vgmlpfnd推荐的最佳循环点（带"!"标记）
- **优先级2**: 选择持续时间最长的循环点（避免短片段）
- **自动过滤**: 忽略命令数<1000的短循环

### VGM自动修剪

- **减少文件大小**: 平均减少40-80%
- **保持音质**: 只包含intro + 一次完整循环
- **设置循环点**: 自动设置VGM头部循环偏移
- **完美兼容**: 所有VGM播放器都能正确循环

---

## 📊 转换效果

### Dragon Quest I & II 测试结果

- **总曲目**: 49首
- **修剪文件**: 11首
- **文件大小**: 13MB → 9.1MB (减少30%)

### 修剪示例

| 曲目 | 原始 | 修剪后 | 减少 |
|------|------|--------|------|
| Ladutorm Castle | 822KB | 165KB | 81% |
| People of the Town | 689KB | 117KB | 84% |
| Fight | 1.1MB | 584KB | 47% |
| King Dragon | 679KB | 349KB | 49% |

---

## 🛠️ 工具说明

### gbs2vgm_final.sh (推荐)
完整自动化流程脚本

```bash
./gbs2vgm_final.sh <m3u文件> <输出目录>
```

### gbs2vgm_batch.exe
主转换程序

```bash
./gbs2vgm_batch.exe <m3u文件> <输出目录>
```

### auto_trim_smart.sh
智能修剪脚本

```bash
./auto_trim_smart.sh
```

### vgm_trim.exe
VGM修剪工具

```bash
./vgm_trim.exe <输入.vgm> 0 <循环起始> <循环结束> <输出.vgm>
```

### vgm_parser.exe
VGM文件信息查看器

```bash
./vgm_parser.exe <文件.vgm>
```

### cleanup_vgm.sh
清理临时文件

```bash
./cleanup_vgm.sh [目录]
```

---

## 🐛 常见问题

### 问题1: 编译失败

**解决**:
```bash
# 在MSYS2中安装依赖
pacman -S gcc make zlib-devel

# 清理并重新编译
make clean
make
make utils
```

### 问题2: 未找到循环点

**原因**:
- 循环模式太复杂
- 曲目本身不循环

**解决**:
- 检查M3U文件中的loop_count设置
- 查看loop_report.txt了解详情

### 问题3: Bank警告

**现象**: "Bank 112 out of range (0-8)!"

**影响**: 无，可以忽略

---

## 📝 更新日志

### v3.0 (2026-02-05)
- ✨ 发布完整版本
- ✨ 整理所有源码和依赖
- ✨ 创建标准发布包
- 📚 完善所有文档

### v2.3 (2026-02-05)
- ✨ 智能循环点选择算法
- ✨ VGM自动修剪功能
- ✨ 循环点自动设置
- ✨ 一键完整流程

### v2.2 (2026-02-04)
- 🐛 修复整数溢出bug
- ✨ 支持M3U旧格式
- ✨ 集成vgmlpfnd

---

## 🙏 致谢

- **gbsplay** - GBS播放引擎
- **vgmtools** - VGM工具集
- **Game Music Emu** - M3U格式参考
- **Denjhang** - 项目发起人、测试

---

## 📄 许可证

GNU GPL v1 or later

---

**版本**: v3.0
**发布日期**: 2026-02-05
**维护者**: Claude Sonnet 4.5 & Denjhang

**感谢使用GBS2VGM！** 🎵
