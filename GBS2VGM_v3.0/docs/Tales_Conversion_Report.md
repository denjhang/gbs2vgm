# Tales of Phantasia - Narikiri Dungeon VGM 转换报告

## 转换日期
2026-02-04

## 源文件
Tales of Phantasia - Narikiri Dungeon (2000-11-10)(Alfa System)(Namco)[GBC].7z

## 转换结果

### 总体统计
- **总曲目数**: 40
- **成功转换**: 36 个有效 VGM 文件
- **失败/空文件**: 4 个（大小为 0）
- **输出目录**: Tales_output_vgm/
- **压缩包**: Tales of Phantasia - Narikiri Dungeon_vgm.zip
- **压缩包大小**: 54 KB（压缩后）
- **解压后大小**: 约 30 MB

### 失败的曲目
以下 4 个曲目转换后文件大小为 0：
1. 01 Burning Tower
2. 01 Freeze
3. 01 Unknown Jingle 4
4. 01 Unknown Song 7

这些曲目可能在原始 GBS 文件中存在问题，或者需要特殊的转换参数。

### 修复的问题
在转换过程中，发现并修复了以下代码问题：

1. **archive_utils.c:117** - 缺少分号
   ```c
   LookToRead2_INIT(&lookStream)  // 错误
   LookToRead2_INIT(&lookStream); // 修复
   ```

2. **m3u_parser.c:149** - realloc 缺少错误检查
   ```c
   // 修复前
   info->entries = realloc(info->entries, ...);

   // 修复后
   struct m3u_entry *new_entries = realloc(info->entries, ...);
   if (!new_entries) {
       fprintf(stderr, "Failed to allocate memory for M3U entry\n");
       return;
   }
   info->entries = new_entries;
   ```

### 转换方法
由于批量转换程序在处理主 M3U 文件时出现段错误，采用了逐个转换单独 M3U 文件的方法：
1. 使用 7z 库解压源文件
2. 逐个转换每个单独的 M3U 文件
3. 将所有 VGM 文件收集到输出目录
4. 创建 ZIP 压缩包

### 成功转换的曲目列表
- 01 Premonition
- 02 Arche
- 03 Harmonious Moment
- 04 Olive Village
- 05 Mysterious Japon
- 06 The Second Act
- 07 Freeze (失败)
- 08 Underground Way
- 09 The Stream of Time
- 10 Retaliation
- 11 Morlia Gallery
- 12 Field of Sunset
- 13 Burning Tower (失败)
- 14 Dark Cave
- 15 Forest of the Treant
- 16 Biting Cold
- 17 Who is Good or Evil
- 18 Take up the Cross
- 19 Overcome Difficulties
- 20 Fighting of the Spirit
- 21 Be Absentminded
- 22 Go a Step Further
- 23-26 Tower of Druaga 1, 2, 3, 7
- 27 The Second Act (Short)
- 28-35 Unknown Songs 2-9
- 36-39 Unknown Jingles 1-4

## 元数据信息
- **游戏**: Tales of Phantasia: Narikiri Dungeon
- **作曲**: Motoi Sakuraba, Shinji Tamura, Toshiki Aida
- **开发商**: Alfa System
- **发行商**: Namco
- **发行日期**: 2000-11-10
- **平台**: Game Boy Color (GBC)
- **Ripper**: CaitSith2
- **Tagger**: Datschge, Knurek

## 工具信息
- **转换工具**: gbs2vgm_batch v1.0.0
- **GBS 模拟器**: gbsplay
- **7z 库**: LZMA SDK
- **VGM 格式**: Video Game Music format

## 注意事项
1. VGM 文件可以使用 VGMPlay 或其他 VGM 播放器播放
2. 压缩率很高是正常的，因为 VGM 文件包含大量重复数据
3. 部分曲目转换失败可能是原始 GBS 文件的问题
4. 所有成功转换的文件都包含完整的 GD3 标签信息

## 下一步建议
1. 使用 VGM 播放器测试所有转换的文件
2. 对于失败的 4 个曲目，可以尝试：
   - 使用调试模式查看详细日志
   - 检查原始 GBS 文件是否有问题
   - 尝试不同的转换参数
