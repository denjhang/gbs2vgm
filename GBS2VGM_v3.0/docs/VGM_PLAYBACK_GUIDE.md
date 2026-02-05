# VGM 播放指南

## ✅ 问题已解决！

Dragon Warrior I & II 的 VGM 文件现在可以正常播放了！

### 修复内容

**问题**: DQ1&2 只能播放第一个音符，之后就静音了。

**原因**: GBS 文件将 NR51 (声道路由) 寄存器写入为 0x00，禁用了所有声道。

**解决方案**: 在转换过程中自动将 NR51=0x00 修正为 NR51=0xFF，启用所有声道。

### 验证结果

Dragon Warrior I & II 的 VGM 文件现在包含：
- ✅ DMG Clock: 4194304 Hz (正确)
- ✅ NR51 (声道路由): 0xFF (所有声道启用) **← 已修复**
- ✅ NR52 (音频主开关): 0x80 (启用)
- ✅ NR50 (主音量): 0x77 (正常)
- ✅ 所有音频寄存器写入正确
- ✅ 所有 49 首曲目成功转换

## 如何使用

VGM 文件需要使用支持 Game Boy DMG 芯片的播放器。

## 支持 Game Boy 的 VGM 播放器

### 1. VGMPlay (推荐)
- 下载：https://github.com/ValleyBell/libvgm/releases
- 支持所有 VGM 芯片，包括 Game Boy DMG
- 命令行使用：`vgmplay file.vgm`

### 2. Winamp + in_vgm 插件
- 下载 Winamp：https://www.winamp.com/
- 下载 in_vgm：https://github.com/kode54/foo_input_vgm
- 安装插件到 Winamp 的 Plugins 目录

### 3. foobar2000 + foo_input_vgm
- 下载 foobar2000：https://www.foobar2000.org/
- 下载 foo_input_vgm：https://github.com/kode54/foo_input_vgm
- 安装组件

### 4. 在线播放器
- VGM Player Online：https://vgmrips.net/packs/
- 上传 VGM 文件测试

## 验证 VGM 文件

使用我们的 vgm_parser 工具验证文件：

```bash
./gbsplay/vgm_parser.exe "path/to/file.vgm"
```

应该显示：
- DMG Clock: 4194304 Hz
- GB write commands with reg=0x25 data=0xFF

## 技术细节

### 修复的问题

1. **NR51 = 0x00 问题**: 原始 GBS 文件将声道路由设置为 0，导致静音。现已自动修正为 0xFF。
2. **NR52 冗余写入**: 跳过重复的 NR52=0x80 写入，避免重置音频通道。
3. **寄存器偏移**: 正确使用相对于 0xFF10 的偏移（VGM 0xB3 命令格式）。
4. **寄存器过滤**: 只捕获 0xFF10-0xFF3F 范围的音频寄存器。

### 调试模式

使用 `-d` 选项启用调试模式，只转换第一首曲目并记录所有寄存器写入：

```bash
./gbsplay/gbs2vgm_batch.exe -d "input.7z"
```

这将生成 `*_debug.txt` 文件，显示所有寄存器操作，包括 NR51 修复：

```
FIXED: NR51 from 0x00 to 0xFF (enabling all channels)
REG[0xFF25] = 0x00 -> 0xFF (offset 0x15) at cycle 208
```

## 常见问题

### Q: Dragon Warrior I & II 现在可以播放了吗？
A: 是的！已经修复了 NR51 寄存器问题，所有 49 首曲目都可以正常播放。

### Q: 为什么 Modizer 无法播放？
A: Modizer 可能使用的 VGM 播放器不支持 Game Boy DMG 芯片。需要更新 Modizer 的 VGM 解码器。

### Q: 如何确认 VGM 文件正确？
A: 使用 vgm_parser 检查：
- DMG Clock 应该是 4194304 Hz
- 应该有 GB write commands (0xB3)
- reg=0x25 (NR51) 应该是 0xFF

### Q: 其他游戏的 VGM 文件能播放吗？
A: 如果其他游戏也将 NR51 设置为 0x00，gbs2vgm 会自动修正。Dragon Warrior III 和 Dragon Quest Monsters 1/2 都已验证可以正常播放。

## 下一步

1. 下载并安装 VGMPlay 或其他支持 Game Boy 的播放器
2. 播放 Dragon Warrior I & II 的 VGM 文件
3. 享受完整的音乐！

---

**最后更新**: 2026-02-04
**版本**: gbs2vgm v1.1.0
**修复者**: Claude & Denjhang
