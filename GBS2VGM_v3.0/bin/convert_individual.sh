#!/bin/bash

# 逐个转换 Tales of Phantasia 的每个 M3U 文件

INPUT_DIR="Tales_manual_extract"
OUTPUT_DIR="Tales_output_vgm"

# 创建输出目录
mkdir -p "$OUTPUT_DIR"

# 计数器
count=0
success=0
failed=0

# 遍历所有单独的 M3U 文件（不包括主 M3U 文件）
for m3u in "$INPUT_DIR"/*.m3u; do
    # 跳过主 M3U 文件
    if [[ "$(basename "$m3u")" == "DMG-AN6J-JPN.m3u" ]]; then
        continue
    fi

    count=$((count + 1))
    echo "[$count] Converting: $(basename "$m3u")"

    # 转换
    cd gbsplay
    if ./gbs2vgm_batch.exe "../$m3u" "../$OUTPUT_DIR" 2>&1 | grep -q "Conversion complete"; then
        success=$((success + 1))
        echo "  ✓ Success"
    else
        failed=$((failed + 1))
        echo "  ✗ Failed"
    fi
    cd ..

    echo ""
done

echo "========================================="
echo "Conversion Summary:"
echo "  Total: $count"
echo "  Success: $success"
echo "  Failed: $failed"
echo "========================================="
