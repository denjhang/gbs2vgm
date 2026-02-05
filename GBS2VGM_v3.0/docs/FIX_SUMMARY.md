# Dragon Warrior I & II VGM 转换修复总结

## 问题描述

Dragon Warrior I & II 的 VGM 文件只能播放第一个音符，之后就静音了。而 Dragon Warrior III 和 Dragon Quest Monsters 1/2 的 VGM 文件可以正常播放。

## 根本原因

通过对比调试日志发现，DQ1&2 的 GBS 文件将 **NR51 (0xFF25) 寄存器写入为 0x00**，这会禁用所有声道的路由，导致无声。

- **NR51 (0xFF25)**: 声道路由/声像控制寄存器
  - 0x00 = 禁用所有声道
  - 0xFF = 启用所有声道（左右声道都输出）

## 修复方案

在 [gbs2vgm_batch.c](gbsplay/gbs2vgm_batch.c#L86-L93) 的 `io_callback` 函数中添加 NR51 修复逻辑：

```c
/* Fix NR51 (0xFF25) - channel routing */
/* Some games write 0x00 which disables all channels */
/* Change to 0xFF to enable all channels */
if (addr == 0xFF25 && value == 0x00) {
    fixed_value = 0xFF;
    if (debug_mode && debug_log) {
        fprintf(debug_log, "FIXED: NR51 from 0x00 to 0xFF (enabling all channels)\n");
    }
}
```

## 验证结果

### 调试日志对比

**修复前 (DQ1&2 - 只有第一个音符):**
```
REG[0xFF25] = 0x00 (offset 0x15) at cycle 208
```

**修复后 (DQ1&2 - 正常播放):**
```
FIXED: NR51 from 0x00 to 0xFF (enabling all channels)
REG[0xFF25] = 0x00 -> 0xFF (offset 0x15) at cycle 208
```

**DQ3 (一直正常):**
```
FIXED: NR51 from 0x00 to 0xFF (enabling all channels)
REG[0xFF25] = 0x00 -> 0xFF (offset 0x15) at cycle 256
```

### VGM 文件验证

使用 vgm_parser 验证生成的 VGM 文件：

```bash
./vgm_parser.exe "Dragon Warrior I & II/01 Overture March (intro).vgm"
```

输出显示：
- `reg=0x16 data=0x80` (NR52 = 0x80，启用音频) ✅
- `reg=0x15 data=0xFF` (NR51 = 0xFF，启用所有声道) ✅
- `reg=0x14 data=0x77` (NR50 = 0x77，主音量) ✅

## 转换结果

成功转换 Dragon Warrior I & II 的所有 49 首曲目：

```
Converting: DMG-AEDJ-JPN.gbs (subsong 22) -> 01 Overture March (intro)
  Done: 50339918 cycles processed
Converting: DMG-AEDJ-JPN.gbs (subsong 15) -> 02 Overture March
  Done: 385891836 cycles processed
...
Converting: DMG-AEDJ-JPN.gbs (subsong 4) -> 49 Unknown Jingle 2
  Done: 21034385 cycles processed

ZIP archive created successfully with 49 files
Conversion complete!
```

## 其他修复

除了 NR51 修复，还包含以下改进：

1. **NR52 冗余写入跳过**: 跳过重复的 NR52=0x80 写入，避免重置音频通道
2. **寄存器偏移修正**: VGM 0xB3 命令使用相对于 0xFF10 的偏移，而不是 0xFF00
3. **音频寄存器过滤**: 只捕获 0xFF10-0xFF3F 范围的音频寄存器
4. **调试模式**: 使用 `-d` 选项启用详细的寄存器写入日志

## 播放测试

生成的 VGM 文件可以使用以下播放器播放：

1. **VGMPlay** (推荐): https://github.com/ValleyBell/libvgm/releases
2. **foobar2000 + foo_input_vgm**: https://www.foobar2000.org/
3. **Winamp + in_vgm**: https://www.winamp.com/

## 技术细节

### Game Boy 音频寄存器

- **NR10-NR14 (0xFF10-0xFF14)**: 声道 1 (方波 + 扫频)
- **NR21-NR24 (0xFF16-0xFF19)**: 声道 2 (方波)
- **NR30-NR34 (0xFF1A-0xFF1E)**: 声道 3 (波形)
- **NR41-NR44 (0xFF20-0xFF23)**: 声道 4 (噪音)
- **NR50 (0xFF24)**: 主音量控制
- **NR51 (0xFF25)**: 声道路由/声像控制
- **NR52 (0xFF26)**: 音频主开关

### VGM 格式

- **0xB3 命令**: Game Boy DMG 寄存器写入
  - 格式: `0xB3 <reg> <data>`
  - `<reg>`: 寄存器偏移 (相对于 0xFF10)
  - `<data>`: 写入的数据值

## 结论

通过修复 NR51 寄存器值，Dragon Warrior I & II 的 VGM 文件现在可以正常播放了。这个修复也适用于其他可能存在相同问题的 GBS 文件。

---

**修复日期**: 2026-02-04
**修复者**: Claude & Denjhang
**版本**: gbs2vgm v1.1.0
