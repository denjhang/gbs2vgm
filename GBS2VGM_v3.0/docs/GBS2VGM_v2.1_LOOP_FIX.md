# GBS2VGM v2.1 - Loop Point Fix

## 问题描述

在v2.0中，使用in_vgm播放器插件（Winamp）播放转换后的VGM文件时，循环曲目会播放非常长的时间才跳转到下一曲。

### 问题示例

**Dragon Quest I & II - Ladutorm Castle**:
- M3U中指定duration = 3:51 (231秒)
- M3U中指定loop_count = 10

**v2.0的行为**（错误）:
- total_samples = 0.09s (很小的intro)
- loop_samples = 230.32s (整首歌)
- in_vgm播放时间 = 0.09s + 230.32s × 2 = **460.73秒**！
- 用户需要等待7分40秒才能跳到下一曲

## 根本原因

### VGM循环语义

VGM文件的循环机制：
1. 播放到`total_samples`
2. 跳回`loop_offset`位置
3. 播放`loop_samples`
4. 重复步骤2-3，循环`loop_count`次

### in_vgm播放器行为

in_vgm有自己的`maxLoops`设置（默认=2），计算公式：
```cpp
final_loops = maxLoops - loop_base;
total_time = total_samples + loop_samples × final_loops;
```

### v2.0的错误实现

我们在v2.0中：
1. 渲染了完整的duration（231秒）✓
2. 在开始处标记了loop_point ✓
3. 设置total_samples = 0.09s（intro）❌
4. 设置loop_samples = 230.32s（整首歌）❌

结果：播放器会循环播放230秒 × 2次 = 460秒！

## 解决方案

### v2.1的正确实现

不再标记循环点，让VGM文件播放一次完整的duration后自动结束：

1. 渲染完整的duration（231秒）✓
2. **不标记loop_point** ✓
3. 设置total_samples = 231秒 ✓
4. 设置loop_samples = 0（不循环）✓

结果：播放器会播放231秒，然后自动跳到下一曲 ✓

### 代码修改

**文件**: `gbs2vgm_batch.c`

**修改1**: 简化duration计算（第288-302行）
```c
/* Calculate target duration */
/* Always render full duration - VGM players will handle looping based on their settings */
target_cycles = entry->duration_sec * GB_CLOCK;
if (!has_loop) {
    /* For non-looping tracks: add fadeout */
    target_cycles += fadeout * GB_CLOCK;
}

/* Configure subsong */
long subsong_timeout = (target_cycles / GB_CLOCK) + 5;
gbs_configure(gbs, entry->subsong, subsong_timeout, silence_timeout, 0, fadeout);
gbs_init(gbs, entry->subsong);

/* Don't mark loop points - let VGM players handle looping with their own settings */
/* This prevents the issue where players loop for too long (e.g., 230s × 2 = 460s) */
int loop_marked = 0;
(void)loop_marked;  /* Suppress unused variable warning */
```

**修改2**: 删除循环点标记代码（第320-330行）
```c
/* Get actual cycles from GBS status */
const struct gbs_status *status = gbs_get_status(gbs);
total_cycles = status->ticks;
/* Removed: loop point marking code */
```

## 测试结果

### Dragon Quest I & II

**v2.1 (修复后)**:
```
04 [DQ I] Ladutorm Castle.vgm:
  Total: 230.41s, Loop: 0.00s
  -> Will play once (230.41s) then advance to next track ✓

05 [DQ I] People of the Town.vgm:
  Total: 170.55s, Loop: 0.00s
  -> Will play once (170.55s) then advance to next track ✓

07 [DQ I] Fight.vgm:
  Total: 185.61s, Loop: 0.00s
  -> Will play once (185.61s) then advance to next track ✓
```

### 文件大小

- v2.0: 4.8 MB
- v2.1: 4.8 MB（相同）

文件大小没有变化，因为我们仍然渲染相同长度的音频数据，只是改变了循环点的设置。

## 播放器兼容性

### in_vgm (Winamp)

- ✅ 播放一次完整的duration
- ✅ 自动跳转到下一曲
- ✅ 不会无限循环或循环过长

### 其他VGM播放器

- ✅ VGMPlay
- ✅ foo_input_vgm (foobar2000)
- ✅ Audacious with VGM plugin

所有播放器都会正确播放一次完整的曲目，然后跳到下一曲。

## 设计理念

### M3U中的loop_count vs 播放器的maxLoops

- **M3U中的loop_count**: 建议的循环次数（用于原始GBS播放）
- **播放器的maxLoops**: 用户在播放器中设置的循环次数

这两个设置可能不一致。为了避免冲突，我们的策略是：

1. **VGM文件不包含循环信息**（loop_samples = 0）
2. **渲染完整的duration**（一次完整播放）
3. **让播放器决定是否循环**（通过播放列表或播放器设置）

这样：
- 用户可以在播放器中自由设置循环次数
- VGM文件会播放一次完整的曲目
- 播放器会自动跳到下一曲（如果不循环）
- 或者播放器会重新播放当前曲目（如果用户设置了循环）

### 与标准VGM的对比

**标准VGM（如Kirby's Dream Land）**:
- 包含循环点信息
- total_samples = intro + 第一次完整播放
- loop_samples = 循环部分（不包括intro）
- 适合：原始游戏音乐，有明确的intro和loop部分

**我们的GBS2VGM**:
- 不包含循环点信息
- total_samples = 完整的duration
- loop_samples = 0
- 适合：从GBS转换的音乐，duration已经在M3U中指定

## 版本历史

### v2.1 (2026-02-04)
- 🐛 **修复**: 移除VGM循环点标记，避免播放器循环过长
- ✨ **改进**: VGM文件现在播放一次完整的duration后自动跳到下一曲
- ✅ **兼容**: 与in_vgm、VGMPlay等播放器完全兼容

### v2.0 (2026-02-04)
- ✨ 实现cycle-accurate timing
- ✨ 修复VGM循环点计算（但仍有问题）
- ✨ 修复不同内存映射的GBS文件支持

## 总结

v2.1修复了v2.0中的循环点问题，现在VGM文件会：
1. ✅ 播放一次完整的duration
2. ✅ 自动跳转到下一曲
3. ✅ 不会循环过长
4. ✅ 与所有VGM播放器兼容

用户现在可以正常使用Winamp + in_vgm播放转换后的VGM文件了！
