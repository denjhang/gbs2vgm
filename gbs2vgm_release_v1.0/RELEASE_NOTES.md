# GBS2VGM v1.0.0 发布说明

## 📦 下载

**文件名**: `gbs2vgm_v1.0.0.zip`
**大小**: 1.1 MB
**发布日期**: 2026-02-03

## 🎉 首次发布

这是 GBS2VGM 的首个正式版本！一个功能完整的 Game Boy Sound 到 VGM 格式转换器。

## ✨ 主要特性

### 核心功能
- ✅ **单文件转换** - 快速转换单个 GBS 文件
- ✅ **批量转换** - 支持 M3U 播放列表批量转换
- ✅ **7z/ZIP 支持** - 自动解压和打包
- ✅ **智能元数据** - 从文件名和 GBS 文件自动提取
- ✅ **完整 GD3 标签** - 包含所有元数据信息

### 技术亮点
- 🔧 **修正时钟偏移** - Game Boy DMG 时钟正确写入 0x80 位置
- 🔧 **正确作者信息** - 从 GBS 文件内部读取真实作者名
- 🔧 **智能 M3U 查找** - 优先匹配 GBS 文件名的主 M3U
- 📦 **无外部依赖** - 集成 7z SDK 和 zlib

### 工具集
- `gbs2vgm.exe` - 单文件转换器
- `gbs2vgm_batch.exe` - 批量转换器
- `vgm_parser.exe` - VGM 文件分析器
- `gd3_reader.exe` - GD3 标签读取器
- `test_filename_parser.exe` - 文件名解析测试

## 📋 系统要求

- **操作系统**: Windows 7 或更高版本
- **架构**: x64 (64位)
- **依赖**: 无（所有库已集成）

## 🚀 快速开始

1. 解压 `gbs2vgm_v1.0.0.zip`
2. 进入 `bin` 目录
3. 拖放 7z 文件到 `convert.bat` 或运行：
   ```bash
   gbs2vgm_batch.exe "your_game.7z"
   ```

详细说明请查看 `QUICKSTART.md`

## 📖 文档

- `README.md` - 完整使用手册
- `QUICKSTART.md` - 快速开始指南
- `CHANGELOG.md` - 更新日志
- `VERSION.txt` - 版本信息

## 🎯 使用示例

### 示例 1：转换 7z 压缩包
```bash
gbs2vgm_batch.exe "Dragon Warrior III.7z"
```
输出：`Dragon Warrior III_vgm.zip`

### 示例 2：分析 VGM 文件
```bash
vgm_parser.exe output.vgm
```

### 示例 3：读取 GD3 标签
```bash
gd3_reader.exe output.vgm
```

## 🐛 已知问题

- 临时文件清理功能待完善
- 仅支持 Windows 平台

## 🔮 计划功能

- Linux/macOS 支持
- GUI 界面
- 进度条显示
- 批量处理多个文件

## 📝 更新内容

### 新增
- 完整的 GBS 到 VGM 转换功能
- M3U 播放列表支持
- 7z/ZIP 自动解压和打包
- 智能文件名解析
- 完整 GD3 标签支持

### 修复
- Game Boy DMG 时钟偏移（0x7C → 0x80）
- 作者名获取方式（从 GBS 文件读取）
- M3U 文件查找逻辑

### 改进
- 集成 7z SDK 和 zlib
- 优化元数据提取
- 完善文档和示例

## 🙏 致谢

- **gbsplay** - Game Boy 音频模拟器核心
- **7z SDK** - Igor Pavlov
- **zlib** - Jean-loup Gailly 和 Mark Adler
- **nsf2vgm** - 压缩包处理参考
- **libvgm** - VGM 格式参考

## 📧 反馈

如有问题或建议，请通过 GitHub Issues 反馈。

## 📄 许可证

GNU GPL v1 或更高版本

---

**感谢使用 GBS2VGM！享受 Game Boy 音乐转换的乐趣！🎵**

---

## 📦 包内容

```
gbs2vgm_v1.0.0.zip
├── bin/                    # 可执行文件
│   ├── gbs2vgm.exe
│   ├── gbs2vgm_batch.exe
│   ├── vgm_parser.exe
│   ├── gd3_reader.exe
│   └── test_filename_parser.exe
├── src/                    # 源代码
│   ├── gbs2vgm.c
│   ├── gbs2vgm_batch.c
│   ├── vgm_writer.c/h
│   ├── m3u_parser.c/h
│   ├── filename_parser.c/h
│   ├── archive_utils.c/h
│   ├── 7z/                 # 7z SDK
│   ├── zlib/               # zlib 库
│   └── build_batch.sh      # 编译脚本
├── examples/               # 示例文件
│   ├── dq3_complete.m3u
│   └── test_metadata.m3u
├── docs/                   # 文档
│   ├── README.md
│   └── CHANGELOG.md
├── README.md               # 主文档
├── QUICKSTART.md           # 快速开始
├── CHANGELOG.md            # 更新日志
├── VERSION.txt             # 版本信息
├── STRUCTURE.txt           # 目录结构
└── convert.bat             # 快速转换脚本
```

## 🔗 相关链接

- **gbsplay**: https://github.com/mmitch/gbsplay
- **VGM 格式**: https://vgmrips.net/wiki/VGM_Specification
- **7z SDK**: https://www.7-zip.org/sdk.html
- **zlib**: https://www.zlib.net/

---

**版本**: 1.0.0
**日期**: 2026-02-03
**作者**: Claude & Denjhang
