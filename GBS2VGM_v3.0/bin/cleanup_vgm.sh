#!/bin/bash
# 清理VGM文件夹中的临时和测试文件

TARGET_DIR="${1:-DQ12_VGM_v2.2}"

echo "=== VGM文件夹清理工具 ==="
echo ""
echo "目标目录: $TARGET_DIR"
echo ""

if [ ! -d "$TARGET_DIR" ]; then
    echo "错误: 目录不存在: $TARGET_DIR"
    exit 1
fi

# 统计要删除的文件
trimmed_files=$(find "$TARGET_DIR" -name "*_trimmed.vgm" 2>/dev/null | wc -l)
temp_files=$(find "$TARGET_DIR" -name "*.tmp" -o -name "*.bak" 2>/dev/null | wc -l)

total_files=$((trimmed_files + temp_files))

if [ "$total_files" -eq 0 ]; then
    echo "✓ 目录已经很干净，没有需要清理的文件"
    exit 0
fi

echo "发现以下文件需要清理:"
echo "  *_trimmed.vgm: $trimmed_files 个"
echo "  临时文件: $temp_files 个"
echo "  总计: $total_files 个"
echo ""

# 显示要删除的文件
if [ "$trimmed_files" -gt 0 ]; then
    echo "将删除的_trimmed文件:"
    find "$TARGET_DIR" -name "*_trimmed.vgm" 2>/dev/null | while read file; do
        echo "  - $(basename "$file")"
    done
    echo ""
fi

# 确认删除
read -p "确认删除这些文件? (y/n): " -n 1 -r
echo ""

if [[ ! $REPLY =~ ^[Yy]$ ]]; then
    echo "取消清理"
    exit 0
fi

# 执行删除
deleted=0

# 删除_trimmed文件
if [ "$trimmed_files" -gt 0 ]; then
    find "$TARGET_DIR" -name "*_trimmed.vgm" -delete 2>/dev/null
    deleted=$((deleted + trimmed_files))
fi

# 删除临时文件
if [ "$temp_files" -gt 0 ]; then
    find "$TARGET_DIR" -name "*.tmp" -delete 2>/dev/null
    find "$TARGET_DIR" -name "*.bak" -delete 2>/dev/null
    deleted=$((deleted + temp_files))
fi

echo ""
echo "✓ 清理完成"
echo "  删除文件: $deleted 个"
echo ""

# 显示最终状态
final_count=$(ls -1 "$TARGET_DIR"/*.vgm 2>/dev/null | wc -l)
final_size=$(du -sh "$TARGET_DIR" 2>/dev/null | cut -f1)

echo "最终状态:"
echo "  VGM文件数: $final_count"
echo "  目录大小: $final_size"
echo ""
