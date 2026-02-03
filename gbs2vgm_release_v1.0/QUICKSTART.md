# 快速开始指南

## 安装

1. 解压 `gbs2vgm_v1.0.0.zip` 到任意目录
2. 将 `bin` 目录添加到系统 PATH（可选）

## 5分钟上手

### 示例 1：转换 7z 压缩包

```bash
cd bin
gbs2vgm_batch.exe "Dragon Warrior III.7z"
```

输出：
- `Dragon Warrior III_vgm.zip` - 包含所有转换后的 VGM 文件

### 示例 2：转换 M3U 播放列表

```bash
cd bin
gbs2vgm_batch.exe ../examples/dq3_complete.m3u
```

输出：
- `Dragon Warrior III_vgm/` - 目录包含所有 VGM 文件

### 示例 3：分析 VGM 文件

```bash
cd bin
vgm_parser.exe output.vgm
```

输出：
- VGM 文件的详细信息（版本、时钟、命令等）

### 示例 4：读取 GD3 标签

```bash
cd bin
gd3_reader.exe output.vgm
```

输出：
- 曲目名、游戏名、作者、发行日期等元数据

## 文件名格式建议

为了获得最佳的元数据提取效果，建议使用以下文件名格式：

```
游戏名 [日文名] (发行日期)(开发商)(发行商)(公司)[系统].7z
```

示例：
```
Dragon Warrior III [Dragon Quest III] (2000-12-08)(Chunsoft)(Tose)(Enix)[GBC].7z
```

## M3U 文件格式

创建 M3U 文件时，建议包含以下元数据：

```m3u
# @TITLE       游戏名称
# @COMPOSER    作曲家
# @DATE        发行日期
# @RIPPER      Ripper名称

filename.gbs::GBS,曲目号,曲目名,时长,,循环次数
```

## 常用命令

### 批量转换当前目录所有 7z 文件

Windows (PowerShell):
```powershell
Get-ChildItem *.7z | ForEach-Object { .\gbs2vgm_batch.exe $_.FullName }
```

Windows (CMD):
```cmd
for %f in (*.7z) do gbs2vgm_batch.exe "%f"
```

### 指定输出目录

```bash
gbs2vgm_batch.exe input.7z my_output_folder
```

## 故障排除

### 问题：转换后的 VGM 没有声音
**解决**：确保使用最新版本（v1.0.0+），时钟偏移问题已修复。

### 问题：找不到 M3U 文件
**解决**：确保 7z 包中包含与 GBS 文件同名的 M3U 文件。

### 问题：作者名显示错误
**解决**：最新版本会从 GBS 文件内部读取作者信息。

### 问题：转换速度慢
**说明**：这是正常的，因为需要模拟 Game Boy 硬件来生成音频数据。

## 获取帮助

查看完整文档：
```bash
# 查看 README
notepad ../README.md

# 查看更新日志
notepad ../CHANGELOG.md
```

## 下一步

- 阅读完整的 [README.md](README.md)
- 查看 [examples](examples/) 目录中的示例文件
- 尝试转换你自己的 GBS 文件

---

**祝你使用愉快！🎵**
