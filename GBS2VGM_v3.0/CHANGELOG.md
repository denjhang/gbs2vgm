# Changelog

All notable changes to GBS2VGM will be documented in this file.

## [3.0] - 2026-02-05

### Added
- 发布完整版本包
- 标准化目录结构 (bin/, src/, docs/, examples/)
- 完整的英文和中文文档
- LICENSE文件
- CHANGELOG文件

### Changed
- 整理所有源码到src/目录
- 整理所有可执行文件到bin/目录
- 整理所有文档到docs/目录

## [2.3] - 2026-02-05

### Added
- 智能循环点选择算法
- VGM自动修剪功能
- 循环点自动设置
- gbs2vgm_final.sh - 一键完整流程脚本
- auto_trim_smart.sh - 智能修剪脚本
- merge_trimmed.sh - 文件合并脚本
- cleanup_vgm.sh - 清理工具

### Fixed
- 修复只识别"!"标记循环点的问题
- 现在可以识别所有类型的循环点

### Changed
- 修剪文件数从7个增加到11个
- 文件大小减少从20%提升到30%

## [2.2] - 2026-02-04

### Added
- Makefile编译系统
- vgmlpfnd循环点检测集成
- M3U旧格式支持
- 自动化转换脚本

### Fixed
- 修复整数溢出bug (long → cycles_t/uint64_t)
- 修复长曲目(>2分钟)转换失败问题

### Changed
- 禁用静音检测，避免提前终止

## [2.1] - 2026-02-03

### Removed
- 移除错误的循环点设置代码

## [2.0] - 2026-02-02

### Added
- 初始版本
- GBS到VGM基本转换功能
- M3U播放列表支持
- 循环曲目3倍长度录制

---

## 版本说明

- **v3.0**: 发布版本，完整打包
- **v2.3**: 智能修剪版本
- **v2.2**: Bug修复版本
- **v2.1**: 清理版本
- **v2.0**: 初始版本
