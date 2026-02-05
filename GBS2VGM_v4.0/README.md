# GBS2VGM v4.0

Game Boy Sound到VGM转换工具 - 支持智能循环点检测和批量修剪

## 🎯 v4.0 新特性

### 核心功能
- ✅ **智能循环点检测** - 使用vgmlpfnd自动检测最佳循环点
- ✅ **批量VGM修剪** - 自动修剪VGM文件到最优大小
- ✅ **多文件夹处理** - 支持批量处理多个游戏目录
- ✅ **详细日志记录** - 实时进度追踪和处理日志
- ✅ **智能文件合并** - 自动覆盖修剪后的文件，保留无法修剪的原文件

### 处理流程
1. **循环点检测** - 优先选择标记为"!"的最佳循环点，其次选择最长循环
2. **自动修剪** - 使用vgm_trim根据检测到的循环点修剪文件
3. **文件合并** - 将修剪后的文件覆盖回原目录
4. **统计报告** - 生成详细的处理统计和日志

## 📁 目录结构

```
GBS2VGM_v4.0/
├── bin/                    # 可执行文件
│   ├── gbs2vgm_batch.exe  # GBS到VGM批量转换工具
│   ├── vgm_trim.exe       # VGM修剪工具
│   └── vgmlpfnd.exe       # 循环点检测工具
│
├── gbsplay/                # 源代码
│   ├── gbs2vgm_batch.c    # 批量转换主程序
│   ├── m3u_parser.c       # M3U播放列表解析
│   ├── vgm_writer.c       # VGM文件写入
│   ├── vgm_trim.c         # VGM修剪工具
│   └── vgmlpfnd.c         # 循环点检测工具
│
├── scripts/                # 处理脚本
│   ├── process_vgm_with_log.sh    # 带日志的VGM处理脚本
│   ├── process_vgm_parallel.sh    # 多线程并行处理脚本
│   ├── auto_trim_dq3.sh           # DQ3专用处理脚本
│   ├── auto_trim_dqm1.sh          # DQM1专用处理脚本
│   ├── auto_trim_dqm2.sh          # DQM2专用处理脚本
│   └── auto_trim_tales.sh         # Tales专用处理脚本
│
├── examples/               # 示例文件
│   ├── input/             # 输入示例（GBS文件）
│   │   └── *.7z          # 压缩的GBS源文件
│   └── output/            # 输出示例（VGM文件）
│       └── *.zip         # 处理后的VGM文件
│
├── docs/                   # 文档
│   ├── README_v4.0.md     # 完整技术文档
│   ├── 使用说明_v4.0.md   # 详细使用说明
│   └── CHANGELOG.md       # 版本更新日志
│
├── Makefile               # 编译配置
└── README.md              # 本文件
```

## 🚀 快速开始

### 方式一：使用处理脚本（推荐）

```bash
# 处理单个文件夹
./scripts/process_vgm_with_log.sh DQ3

# 处理多个文件夹
./scripts/process_vgm_with_log.sh DQM1
./scripts/process_vgm_with_log.sh DQM2
./scripts/process_vgm_with_log.sh Tales
```

### 方式二：手动处理

```bash
# 1. 转换GBS到VGM
./bin/gbs2vgm_batch.exe "path/to/game.m3u" "output_folder"

# 2. 检测循环点
cd output_folder
for file in *.vgm; do
    echo "$file" | ../gbsplay/vgmlpfnd.exe -silent
done

# 3. 修剪VGM文件
./bin/vgm_trim.exe input.vgm 0 <loop_start> <loop_end> output.vgm
```

## 📊 处理示例

### 实际处理结果

| 游戏 | 总文件数 | 修剪成功 | 跳过文件 | 平均压缩率 |
|------|----------|----------|----------|------------|
| Dragon Quest III | 64 | 8 | 56 | 83% |
| Dragon Quest Monsters 1 | 28 | 15 | 13 | 82% |
| Dragon Quest Monsters 2 | 31 | 10 | 21 | 80% |
| Tales of Phantasia | 39 | 29 | 10 | 78% |

### 处理日志示例

```
==========================================
=== DQ3 VGM处理脚本 ===
==========================================
开始时间: 2026-02-05 10:00:00
输入目录: working/DQ3_VGM
总文件数: 64

----------------------------------------
[1/64] 处理: 01 Overture.vgm
时间: 10:00:01
  结果: 跳过 - 无有效循环点

----------------------------------------
[2/64] 处理: 02 Intermezzo.vgm
时间: 10:00:02
  循环点: 953 -> 1096704 (命令数: 40888) [最佳!]
  大小: 220KB -> 37KB (减少84%)
  结果: 修剪成功

...

==========================================
=== 全部完成 ===
==========================================
结束时间: 2026-02-05 10:15:00
修剪成功: 8
跳过文件: 56
错误: 0
```

## 🔧 编译

### 前置要求
- GCC编译器
- Make工具
- MSYS2/MinGW（Windows）

### 编译步骤

```bash
# 编译主程序
make

# 编译工具程序
make utils

# 清理编译文件
make clean
```

## 📚 详细文档

- **[完整技术文档](docs/README_v4.0.md)** - 详细的技术说明和API文档
- **[使用说明](docs/使用说明_v4.0.md)** - 图文并茂的使用教程
- **[更新日志](docs/CHANGELOG.md)** - 版本更新历史

## 🎮 支持的游戏

已测试并支持以下Game Boy游戏：
- Dragon Quest I & II
- Dragon Quest III
- Dragon Quest Monsters 1
- Dragon Quest Monsters 2
- Tales of Phantasia

理论上支持所有使用GBS格式的Game Boy游戏。

## 🛠️ 工具说明

### gbs2vgm_batch.exe
批量转换GBS文件到VGM格式
- 支持M3U播放列表
- 自动处理多个子曲目
- 可配置循环次数

### vgmlpfnd.exe
智能循环点检测工具
- 自动分析VGM文件
- 识别最佳循环点（标记为"!"）
- 提供多个候选循环点

### vgm_trim.exe
VGM文件修剪工具
- 根据循环点修剪文件
- 大幅减小文件大小
- 保持音质不变

## 📝 版本历史

### v4.0 (2026-02-05)
- ✨ 新增智能循环点检测功能
- ✨ 新增批量VGM修剪功能
- ✨ 新增多文件夹处理脚本
- ✨ 新增详细日志和进度追踪
- 🐛 修复文件路径处理问题
- 📚 更新文档和示例

### v3.0 (2026-02-04)
- ✨ 重构批量转换工具
- ✨ 改进M3U解析
- 🐛 修复内存泄漏问题

### v2.2 (2026-02-03)
- ✨ 初始版本
- ✨ 基本GBS到VGM转换功能

## 🤝 贡献

欢迎提交问题和改进建议！

## 📄 许可证

GNU GPL v1 or later

## 👥 作者

- **开发**: Claude Sonnet 4.5
- **维护**: Denjhang
- **最后更新**: 2026-02-05

---

**注意**: 本工具仅供学习和研究使用，请尊重原游戏版权。
