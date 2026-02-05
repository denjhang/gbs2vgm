# GBS2VGM v3.0 - 快速开始指南

## 5分钟上手

### 步骤1: 准备环境

**Windows用户**:
- 下载并安装 [MSYS2](https://www.msys2.org/) 或 [Git for Windows](https://git-scm.com/)
- 解压 `GBS2VGM_v3.0.zip`

**Linux/Mac用户**:
- 确保已安装 bash, gcc, make, zlib

### 步骤2: 准备文件

你需要：
- GBS文件 (Game Boy音乐文件)
- M3U播放列表 (可选，用于批量转换)

### 步骤3: 运行转换

打开终端，进入bin目录：

```bash
cd GBS2VGM_v3.0/bin
```

**单个文件转换**:
```bash
./gbs2vgm_batch.exe "path/to/music.gbs" "output"
```

**M3U批量转换** (推荐):
```bash
./gbs2vgm_final.sh "path/to/playlist.m3u" "output"
```

### 步骤4: 查看结果

转换完成后，在输出目录中你会看到：
- `*.vgm` - VGM音乐文件
- `loop_report.txt` - 循环点检测报告

### 步骤5: 播放VGM

使用以下播放器之一：
- **VGMPlay** - 专业VGM播放器
- **foobar2000** - 需要VGM插件
- **Winamp** - 需要in_vgm插件

---

## 示例

### 示例1: 转换单个GBS文件

```bash
# 转换
./gbs2vgm_batch.exe "game.gbs" "output"

# 查看结果
ls -lh output/
```

### 示例2: 使用M3U批量转换

创建M3U文件 `playlist.m3u`:
```
# @TITLE My Game Music
# @COMPOSER Composer Name

game.gbs::GBS,0,Track 1,2:30,,10
game.gbs::GBS,1,Track 2,1:45,,1
game.gbs::GBS,2,Track 3,3:00,,10
```

运行转换：
```bash
./gbs2vgm_final.sh "playlist.m3u" "output"
```

### 示例3: 只修剪现有VGM文件

如果你已经有VGM文件，只想修剪：

```bash
# 将VGM文件放在 DQ12_VGM_v2.2/ 目录
# 运行智能修剪
./auto_trim_smart.sh

# 查看修剪后的文件
ls -lh DQ12_VGM_v2.2_trimmed_smart/
```

---

## M3U格式说明

### 基本格式

```
# 注释行以#开头
# @TITLE 专辑标题
# @COMPOSER 作曲家

文件名::GBS,子歌曲号,标题,时长,,循环次数
```

### 参数说明

- **文件名**: GBS文件路径
- **子歌曲号**: 从0开始的曲目编号
- **标题**: 曲目名称
- **时长**: 格式为 `分:秒` (如 `2:30`)
- **循环次数**: 
  - `1` = 非循环曲目
  - `10` = 循环曲目

### 示例

```
# Dragon Quest I & II
# @TITLE Dragon Quest I & II
# @COMPOSER Koichi Sugiyama

dq12.gbs::GBS,0,Overture March,1:29,,1
dq12.gbs::GBS,1,Ladutorm Castle,3:51,,10
dq12.gbs::GBS,2,People of the Town,2:51,,10
```

---

## 常见问题

### Q: 转换后的文件很大？

A: 循环曲目会录制3倍长度用于循环检测。使用 `gbs2vgm_final.sh` 会自动修剪。

### Q: 如何知道哪些文件被修剪了？

A: 查看 `loop_report.txt` 文件，带"!"标记的是找到循环点的曲目。

### Q: 播放器不循环？

A: 确保使用了修剪功能。未修剪的文件不会设置循环点。

### Q: 编译失败？

A: 确保安装了所有依赖：
```bash
# MSYS2
pacman -S gcc make zlib-devel

# Ubuntu/Debian
sudo apt-get install gcc make zlib1g-dev
```

---

## 下一步

- 阅读 [README.md](README.md) 了解完整功能
- 阅读 [README_CN.md](README_CN.md) 中文文档
- 查看 [docs/](docs/) 目录获取详细文档
- 查看 [examples/](examples/) 目录获取示例文件

---

**祝你使用愉快！** 🎵

如有问题，请查看文档或提交Issue。
