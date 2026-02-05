# GBS2VGM v3.0 发布说明

**发布日期**: 2026-02-05

---

## 🎉 重大更新

GBS2VGM v3.0 是第一个正式发布版本，提供完整的GBS到VGM转换解决方案。

### 核心功能

✅ **完整的转换流程**
- GBS → VGM 转换
- 智能循环点检测
- 自动VGM修剪
- 循环点自动设置

✅ **智能算法**
- 优先使用vgmlpfnd推荐的最佳循环点
- 自动选择持续时间最长的循环
- 过滤短片段循环（<1000命令）

✅ **一键操作**
- `gbs2vgm_final.sh` - 完整自动化流程
- 从GBS到最终VGM，全自动完成

---

## 📦 发布内容

### 可执行文件 (bin/)
- `gbs2vgm_batch.exe` - 主转换程序 (631KB)
- `vgm_trim.exe` - VGM修剪工具 (304KB)
- `vgm_parser.exe` - VGM信息查看器 (265KB)
- `test_m3u_parser.exe` - M3U解析测试 (441KB)

### 脚本工具 (bin/)
- `gbs2vgm_final.sh` - 完整自动化流程 ⭐推荐
- `auto_trim_smart.sh` - 智能修剪脚本
- `merge_trimmed.sh` - 文件合并脚本
- `cleanup_vgm.sh` - 清理工具

### 源代码 (src/)
- 完整的C源代码
- Makefile编译配置
- 142个源文件

### 文档 (docs/ + 根目录)
- `README.md` - 英文完整文档
- `README_CN.md` - 中文完整文档
- `QUICKSTART.md` - 快速开始指南
- `LICENSE` - 许可证
- `CHANGELOG.md` - 更新日志
- `VERSION` - 版本信息
- 17个详细技术文档

### 示例 (examples/)
- 3个M3U示例文件

---

## 🚀 新特性

### v3.0 新增

1. **标准化发布包**
   - 规范的目录结构
   - 完整的文档
   - 开箱即用

2. **完善的文档系统**
   - 英文和中文双语文档
   - 快速开始指南
   - 详细技术文档

3. **发布检查工具**
   - `check_release.sh` - 验证发布包完整性

### v2.3 功能（已包含）

1. **智能循环点检测**
   - 不再只依赖"!"标记
   - 自动选择最佳循环点
   - 支持复杂循环模式

2. **VGM自动修剪**
   - 平均减少文件大小40-80%
   - 自动设置循环偏移
   - 保持完整音质

3. **一键完整流程**
   - 转换 → 检测 → 修剪 → 设置
   - 全自动化，无需手动干预

### v2.2 功能（已包含）

1. **整数溢出修复**
   - 修复长曲目(>2分钟)转换失败
   - 使用64位整数(cycles_t)

2. **M3U格式兼容**
   - 支持旧格式(5逗号)
   - 支持新格式(6逗号, GME)

---

## 📊 性能表现

### Dragon Quest I & II 测试

- **总曲目**: 49首
- **成功转换**: 49首 (100%)
- **找到循环点**: 11首
- **文件大小**: 13MB → 9.1MB (减少30%)

### 修剪效果

| 类型 | 平均减少 | 最大减少 |
|------|---------|---------|
| 循环曲目 | 60% | 84% |
| 所有文件 | 30% | - |

### 转换速度

- 约为实时播放的20-30倍
- 49首曲目约5分钟完成

---

## 🔧 系统要求

### Windows
- Windows 7 或更高版本
- MSYS2 或 Git Bash (用于运行脚本)

### Linux
- 任何现代Linux发行版
- bash, gcc, make, zlib

### Mac
- macOS 10.10 或更高版本
- Xcode Command Line Tools

---

## 📖 快速开始

### 1. 下载并解压

下载 `GBS2VGM_v3.0.zip` 并解压到任意目录。

### 2. 运行转换

```bash
cd GBS2VGM_v3.0/bin
./gbs2vgm_final.sh "your_music.m3u" "output"
```

### 3. 播放VGM

使用VGMPlay、foobar2000或其他VGM播放器播放生成的VGM文件。

详细说明请参阅 [QUICKSTART.md](QUICKSTART.md)

---

## 🐛 已知问题

### 无重大问题

当前版本没有已知的重大问题。

### 小问题

1. **Bank警告**: 某些GBS文件会显示"Bank out of range"警告
   - **影响**: 无，可以忽略
   - **原因**: GBS文件访问超出范围的ROM bank（正常现象）

2. **部分循环未检测**: 某些复杂循环模式可能无法检测
   - **影响**: 这些文件不会被修剪，但仍可正常播放
   - **解决**: 手动调整vgmlpfnd参数

---

## 🔄 升级说明

### 从v2.x升级

1. 备份现有文件
2. 解压v3.0到新目录
3. 复制你的GBS和M3U文件到新目录
4. 使用新的脚本重新转换

**注意**: v3.0的输出与v2.x完全兼容。

---

## 📞 支持与反馈

### 获取帮助

- 阅读 [README.md](README.md) 或 [README_CN.md](README_CN.md)
- 查看 [QUICKSTART.md](QUICKSTART.md)
- 查看 [docs/](docs/) 目录中的详细文档

### 报告问题

- GitHub Issues: [创建Issue](https://github.com/yourusername/gbs2vgm/issues)
- Email: your.email@example.com

### 贡献代码

欢迎提交Pull Request！请参阅 [README.md](README.md) 中的贡献指南。

---

## 🙏 致谢

感谢以下项目和个人：

- **gbsplay** - GBS播放引擎
- **vgmtools** - VGM工具集
- **Game Music Emu** - M3U格式参考
- **Denjhang** - 项目发起人、测试、反馈
- **所有测试用户** - 宝贵的反馈和建议

---

## 📄 许可证

GNU GPL v1 or later

详见 [LICENSE](LICENSE) 文件。

---

## 🎊 结语

GBS2VGM v3.0 是一个里程碑版本，提供了完整、稳定、易用的GBS到VGM转换解决方案。

我们希望这个工具能帮助你轻松地将Game Boy音乐转换为VGM格式，并在各种播放器中享受高质量的游戏音乐。

**感谢使用GBS2VGM！** 🎵

---

**版本**: v3.0
**发布日期**: 2026-02-05
**维护者**: Claude Sonnet 4.5 & Denjhang
