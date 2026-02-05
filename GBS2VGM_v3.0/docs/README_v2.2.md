# GBS2VGM v2.2 - 自动循环检测版本

## 概述

GBS2VGM v2.2 实现了完整的GBS到VGM转换流程,包括:
1. ✅ 自动检测循环曲目(loop_count=10)和非循环曲目(loop_count=1)
2. ✅ 对循环曲目录制3倍长度,用于循环点检测
3. ✅ 使用vgmlpfnd自动检测循环点
4. ⏳ VGM文件修剪(待实现)

## 主要修复

### v2.2 (2026-02-04)
- **修复整数溢出bug**: 将`target_cycles`从`long`改为`cycles_t`(uint64_t)
  - 问题: 231秒 × 4194304 Hz × 3 = 2,906,652,672 超过32位整数最大值
  - 影响: 导致某些长曲目(>2分钟)转换失败,生成空VGM文件
- **禁用静音检测**: 将`silence_timeout`设置为0
  - 避免某些曲目因开头静音而被提前终止
- **添加Makefile**: 规范化编译流程
- **支持M3U旧格式**: 兼容5逗号格式(无intro-loop字段)

### v2.1
- 移除了错误的循环点设置

### v2.0
- 初始版本,包含错误的循环点计算

## 编译

```bash
make              # 编译主程序
make test         # 编译并运行测试
make utils        # 编译工具程序
make clean        # 清理
```

## 使用方法

### 方法1: 使用自动转换脚本

```bash
./convert_dq12.sh
```

这会:
1. 转换Dragon Quest I & II的所有曲目
2. 对每个VGM文件运行循环点检测
3. 生成循环点报告(`loop_report.txt`)

### 方法2: 手动转换

```bash
# 步骤1: 转换GBS到VGM
./gbs2vgm_batch.exe "path/to/file.m3u" "output_dir"

# 步骤2: 检测循环点
cd output_dir
for vgm in *.vgm; do
    echo "$vgm" | ../gbsplay/vgmlpfnd.exe -silent > "${vgm%.vgm}_loop.txt"
done
```

## M3U格式

支持两种格式:

### 旧格式 (5个逗号)
```
filename.gbs::GBS,subsong,title,duration,,loop_count
```

示例:
```
DMG-AEDJ-JPN.gbs::GBS,16,[DQ I] Ladutorm Castle,3:51,,10
```

### 新格式 (6个逗号, GME兼容)
```
filename.gbs::GBS,subsong,title,duration,intro-loop,fade,loop_count
```

示例:
```
CGB-BD3J-JPN.gbs::GBS,0,Overture,1:39,0:10-,3,10
```

## 循环检测

vgmlpfnd输出格式:
```
Source Block		  Block Copy		Copy Information
Start	  Time		Start	  Time		Cmds	Time
24496	00:00.56	5114858	01:55.98	5110	00:21.33
1061612	00:24.07  !	6151996	02:19.50	147219	09:11.73
```

标记为`!`的是最佳循环点:
- 循环开始: 1061612 (00:24.07)
- 循环结束: 6151996 (02:19.50)
- 循环长度: 147219命令 (09:11.73)

## 已知问题

1. **VGM文件包含3倍长度数据**
   - 当前版本不会自动修剪VGM文件
   - 文件大小约为最终大小的3倍
   - 播放器会正确播放,但文件较大

2. **循环点未自动设置**
   - vgmlpfnd只检测循环点,不修改VGM文件
   - 需要手动使用循环点信息修剪VGM

3. **Bank警告**
   - 某些GBS文件会显示"Bank 112 out of range"警告
   - 这是正常的,不影响转换结果

## 下一步计划

1. **实现VGM修剪工具** (`vgm_trim.c`)
   - 解析VGM命令流
   - 根据循环点修剪数据
   - 更新VGM头部的循环偏移

2. **集成到gbs2vgm_batch**
   - 自动调用vgmlpfnd
   - 自动修剪VGM文件
   - 一键完成完整流程

3. **优化循环检测**
   - 支持自定义循环检测参数
   - 改进循环点选择算法

## 技术细节

### 整数溢出修复

**问题代码**:
```c
long target_cycles = entry->duration_sec * GB_CLOCK * render_loops;
// 231 * 4194304 * 3 = 2,906,652,672 (溢出!)
```

**修复后**:
```c
cycles_t target_cycles = (cycles_t)entry->duration_sec * GB_CLOCK * render_loops;
// 使用uint64_t,支持最大值: 18,446,744,073,709,551,615
```

### 循环策略

- `loop_count = 1`: 非循环曲目
  - 录制: 1× duration + 3秒fadeout
  - 示例: 片头曲、音效

- `loop_count > 1`: 循环曲目
  - 录制: 3× duration
  - 用于循环点检测
  - 示例: 战斗音乐、城镇音乐

## 参考

- [Game Music Emu (GME)](https://bitbucket.org/mpyne/game-music-emu/)
- [VGM文件格式规范](http://vgmrips.net/wiki/VGM_Specification)
- [gbsplay](https://github.com/mmitch/gbsplay)
- [vgmtools](https://github.com/vgmrips/vgmtools)

## 许可证

GNU GPL v1 or later

## 作者

- Claude Sonnet 4.5 (AI Assistant)
- Denjhang (用户/测试)
- 基于gbsplay和vgmtools项目
