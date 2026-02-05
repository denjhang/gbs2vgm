#!/bin/bash
# 合并修剪后的VGM文件到原文件夹
# 用法: ./merge_trimmed.sh

SOURCE_DIR="DQ12_VGM_v2.2"
TRIMMED_DIR="DQ12_VGM_v2.2_trimmed_smart"
BACKUP_DIR="DQ12_VGM_v2.2_backup"

echo "=== 合并修剪后的VGM文件 ==="
echo ""
echo "源目录: $SOURCE_DIR"
echo "修剪目录: $TRIMMED_DIR"
echo "备份目录: $BACKUP_DIR"
echo ""

# 检查目录是否存在
if [ ! -d "$SOURCE_DIR" ]; then
    echo "错误: 源目录不存在: $SOURCE_DIR"
    exit 1
fi

if [ ! -d "$TRIMMED_DIR" ]; then
    echo "错误: 修剪目录不存在: $TRIMMED_DIR"
    exit 1
fi

# 创建备份
echo "创建备份..."
if [ -d "$BACKUP_DIR" ]; then
    echo "警告: 备份目录已存在，将被覆盖"
    rm -rf "$BACKUP_DIR"
fi

cp -r "$SOURCE_DIR" "$BACKUP_DIR"
echo "✓ 备份完成: $BACKUP_DIR"
echo ""

# 合并修剪后的文件
echo "合并修剪后的文件..."
merged_count=0

for trimmed_file in "$TRIMMED_DIR"/*.vgm; do
    if [ -f "$trimmed_file" ]; then
        filename=$(basename "$trimmed_file")
        dest_file="$SOURCE_DIR/$filename"

        # 复制修剪后的文件，覆盖原文件
        cp "$trimmed_file" "$dest_file"

        # 获取文件大小
        trimmed_size=$(stat -c%s "$trimmed_file" 2>/dev/null || stat -f%z "$trimmed_file" 2>/dev/null)

        echo "  合并: $filename ($(numfmt --to=iec-i --suffix=B $trimmed_size 2>/dev/null || echo "$trimmed_size bytes"))"

        merged_count=$((merged_count + 1))
    fi
done

echo ""
echo "✓ 合并完成"
echo "  合并文件数: $merged_count"
echo ""

# 清理修剪目录
read -p "是否删除修剪目录? (y/n): " -n 1 -r
echo ""
if [[ $REPLY =~ ^[Yy]$ ]]; then
    rm -rf "$TRIMMED_DIR"
    echo "✓ 已删除修剪目录"
fi

echo ""
echo "=== 完成 ==="
echo ""
echo "最终目录: $SOURCE_DIR"
echo "备份目录: $BACKUP_DIR (如需恢复，可以删除$SOURCE_DIR并重命名此目录)"
echo ""
