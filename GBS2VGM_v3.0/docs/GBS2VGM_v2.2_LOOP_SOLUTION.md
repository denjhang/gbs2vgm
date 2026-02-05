# GBS2VGM v2.2 - 正确的循环点支持

## 问题分析

用户指出了v2.1的根本问题：**大部分游戏音乐应该包含循环点，让播放器可以无限循环**。

v2.1移除了循环点，导致VGM文件播放一次就结束，这是错误的。

## 正确的解决方案

参考GME (Game Music Emu)的M3U格式，支持**intro和loop时间**：

### GME M3U格式

```
filename,track,name,length,intro-loop,fade,repeat
```

**字段说明**：
1. `filename` - GBS文件名
2. `track` - 子曲目编号
3. `name` - 曲目名称
4. `length` - 总时长
5. `intro-loop` - intro和loop信息：
   - `-` : 整首歌都是循环（intro=0, loop=length）
   - `0:30-` : intro=0:30, loop=length-0:30
   - `0:30-2:30` : intro=0:30, loop=2:30
   - `2:30` : intro=0, loop=2:30
6. `fade` - 淡出时长
7. `repeat` - 循环次数（建议值）

### 示例

```m3u
# @TITLE       Dragon Quest I & II
# @COMPOSER    Koichi Sugiyama

# 整首歌都是循环
DMG-AEDJ-JPN.gbs::GBS,16,[DQ I] Ladutorm Castle,3:51,-,3,10

# 有intro的循环
DMG-AEDJ-JPN.gbs::GBS,11,[DQ I] People of the Town,2:51,0:10-,3,10

# 不循环的曲目（intro、jingle等）
DMG-AEDJ-JPN.gbs::GBS,22,Overture March (intro),0:09,,3,1
```

## VGM循环点设置

根据intro/loop信息，正确设置VGM头部：

### 情况1：整首歌都是循环 (intro=0, loop=length)

```
录制：1次duration (3:51)
total_samples = duration (3:51)
loop_samples = duration (3:51)
loop_offset = 开始处（或很小的offset）

播放行为：
- 播放3:51
- 跳回开始
- 再播放3:51 × (maxLoops-1)次
- 总时间 = 3:51 × maxLoops
```

### 情况2：有intro的循环 (intro=0:10, loop=2:41)

```
录制：intro + loop (0:10 + 2:41 = 2:51)
total_samples = intro + loop (2:51)
loop_samples = loop (2:41)
loop_offset = intro结束的位置

播放行为：
- 播放2:51（intro + 第1次loop）
- 跳回intro结束位置
- 再播放2:41 × (maxLoops-1)次
- 总时间 = 0:10 + 2:41 × maxLoops
```

### 情况3：不循环 (loop_count=1)

```
录制：1次duration + fadeout
total_samples = duration + fadeout
loop_samples = 0
loop_offset = 0

播放行为：
- 播放一次
- 结束，跳到下一曲
```

## 实现步骤

### 1. 更新M3U解析器 ✅

已完成：
- 添加intro_sec, intro_ms, loop_sec, loop_ms字段到m3u_entry
- 解析GME格式的intro-loop字段
- 支持多种格式：`-`, `intro-`, `intro-loop`, `loop`

### 2. 更新VGM转换逻辑 (待完成)

需要修改gbs2vgm_batch.c：

```c
if (entry->loop_sec >= 0) {
    /* 有循环信息 */
    if (entry->intro_sec > 0) {
        /* 情况2：有intro的循环 */
        /* 录制：intro + loop */
        target_cycles = (entry->intro_sec + entry->loop_sec) * GB_CLOCK;
        /* 在intro结束处标记循环点 */
        loop_point_cycles = entry->intro_sec * GB_CLOCK;
    } else {
        /* 情况1：整首歌都是循环 */
        /* 录制：1次duration */
        target_cycles = entry->duration_sec * GB_CLOCK;
        /* 在开始处标记循环点 */
        loop_point_cycles = GB_CLOCK / 10;  /* ~0.1秒后 */
    }
} else {
    /* 情况3：没有循环信息（不循环或使用旧格式） */
    if (entry->loop_count > 1) {
        /* 假设整首歌都是循环 */
        target_cycles = entry->duration_sec * GB_CLOCK;
        loop_point_cycles = GB_CLOCK / 10;
    } else {
        /* 不循环 */
        target_cycles = (entry->duration_sec + fadeout) * GB_CLOCK;
        loop_point_cycles = -1;  /* 不标记循环点 */
    }
}
```

### 3. 更新M3U文件 (需要手动或工具)

当前的M3U文件格式：
```
DMG-AEDJ-JPN.gbs::GBS,16,[DQ I] Ladutorm Castle,3:51,,10
```

需要更新为：
```
DMG-AEDJ-JPN.gbs::GBS,16,[DQ I] Ladutorm Castle,3:51,-,3,10
```

添加intro-loop字段（`-`表示整首歌都是循环）。

## 与in_vgm的兼容性

in_vgm的maxLoops设置（默认=2）：

### 情况1：整首歌循环
- VGM: total=3:51, loop=3:51
- 播放：3:51 × 2 = 7:42

### 情况2：有intro的循环
- VGM: total=2:51, loop=2:41
- 播放：2:51 + 2:41 × 1 = 5:32

### 情况3：不循环
- VGM: total=0:09, loop=0
- 播放：0:09

这样用户可以在播放器中设置maxLoops来控制循环次数，同时VGM文件包含正确的循环点信息。

## 优势

1. ✅ **支持无限循环** - 游戏音乐可以正确循环
2. ✅ **支持intro** - 可以正确处理有intro的曲目
3. ✅ **播放器控制** - 用户可以在播放器中设置循环次数
4. ✅ **文件大小合理** - 只录制intro + 1次loop
5. ✅ **兼容GME格式** - 使用标准的M3U格式

## 下一步

1. 完成gbs2vgm_batch.c的循环点逻辑实现
2. 创建工具来更新现有的M3U文件，添加intro-loop信息
3. 测试各种情况的VGM文件
4. 验证与in_vgm、VGMPlay等播放器的兼容性

## 参考

- GME M3U格式：https://github.com/mcfiredrill/game-music-emu
- libvgm循环点实现
- VGM v1.71规范
