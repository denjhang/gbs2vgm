#!/bin/bash
# GBS2VGM Complete Workflow - 完整自动化流程（智能版）
# 功能: GBS转换 → 循环点检测 → 智能VGM修剪 → 合并到原文件夹
#
# 使用方法:
#   ./gbs2vgm_final.sh <m3u文件> <输出目录>
#
# 示例:
#   ./gbs2vgm_final.sh "path/to/file.m3u" "DQ12_Final"

set -e  # 遇到错误立即退出

# 检查参数
if [ $# -lt 2 ]; then
    echo "用法: $0 <M3U文件> <输出目录>"
    echo ""
    echo "示例:"
    echo "  $0 \"Dragon Quest.m3u\" \"DQ12_Final\""
    exit 1
fi

M3U_FILE="$1"
OUTPUT_DIR="$2"
TEMP_DIR="${OUTPUT_DIR}_temp"

echo "========================================"
echo "  GBS2VGM 完整自动化流程 v2.3 (智能版)"
echo "========================================"
echo ""
echo "M3U文件: $M3U_FILE"
echo "输出目录: $OUTPUT_DIR"
echo ""

# 检查M3U文件是否存在
if [ ! -f "$M3U_FILE" ]; then
    echo "错误: M3U文件不存在: $M3U_FILE"
    exit 1
fi

# 检查必要的程序
if [ ! -f "gbs2vgm_batch.exe" ]; then
    echo "错误: gbs2vgm_batch.exe 不存在，请先运行 'make'"
    exit 1
fi

if [ ! -f "gbsplay/vgmlpfnd.exe" ]; then
    echo "错误: vgmlpfnd.exe 不存在"
    exit 1
fi

if [ ! -f "vgm_trim.exe" ]; then
    echo "错误: vgm_trim.exe 不存在，请先运行 'make utils'"
    exit 1
fi

# 步骤1: GBS转换到VGM
echo "========================================"
echo "步骤 1/3: GBS → VGM 转换"
echo "========================================"
echo ""

# 创建临时目录
mkdir -p "$TEMP_DIR"

# 运行转换
./gbs2vgm_batch.exe "$M3U_FILE" "$TEMP_DIR"

if [ $? -ne 0 ]; then
    echo "错误: GBS转换失败"
    exit 1
fi

echo ""
echo "✓ 转换完成"
echo ""

# 步骤2: 智能循环点检测和修剪
echo "========================================"
echo "步骤 2/3: 智能循环点检测和修剪"
echo "========================================"
echo ""

# 创建最终输出目录
mkdir -p "$OUTPUT_DIR"

trimmed_count=0
copied_count=0

# Process each VGM file
for vgm_file in "$TEMP_DIR"/*.vgm; do
    if [ ! -f "$vgm_file" ]; then
        continue
    fi

    filename=$(basename "$vgm_file")
    output_file="$OUTPUT_DIR/$filename"

    echo "处理: $filename"

    # Run vgmlpfnd and capture output
    loop_data=$(echo "$vgm_file" | ./gbsplay/vgmlpfnd.exe -silent 2>/dev/null)

    if [ -z "$loop_data" ]; then
        echo "  复制: 无循环点"
        cp "$vgm_file" "$output_file"
        copied_count=$((copied_count + 1))
        continue
    fi

    # Parse loop data and find best loop point
    best_loop_start=""
    best_loop_end=""
    best_loop_duration=0
    has_exclamation=false

    while IFS= read -r line; do
        # Skip header lines
        if [[ $line =~ ^Source || $line =~ ^Start ]]; then
            continue
        fi

        # Check for loop point with "!" (highest priority)
        if [[ $line =~ ^([0-9]+)[[:space:]]+[0-9:.]+[[:space:]]+![[:space:]]+([0-9]+)[[:space:]]+[0-9:.]+[[:space:]]+([0-9]+) ]]; then
            loop_start="${BASH_REMATCH[1]}"
            loop_end="${BASH_REMATCH[2]}"
            loop_cmds="${BASH_REMATCH[3]}"

            best_loop_start="$loop_start"
            best_loop_end="$loop_end"
            has_exclamation=true
            echo "  循环: $loop_start → $loop_end (最佳, 命令数: $loop_cmds)"
            break
        fi

        # Check for loop point with "e" (end of file) or regular loop point
        if [[ $line =~ ^([0-9]+)[[:space:]]+[0-9:.]+[[:space:]]+(e)?[[:space:]]*([0-9]+)[[:space:]]+[0-9:.]+[[:space:]]+([0-9]+) ]]; then
            loop_start="${BASH_REMATCH[1]}"
            loop_end="${BASH_REMATCH[3]}"
            loop_cmds="${BASH_REMATCH[4]}"

            # Calculate loop duration (use command count as proxy)
            # Only consider loops with reasonable duration (> 1000 commands)
            if [ "$loop_cmds" -gt 1000 ] && [ "$loop_cmds" -gt "$best_loop_duration" ]; then
                best_loop_start="$loop_start"
                best_loop_end="$loop_end"
                best_loop_duration="$loop_cmds"
            fi
        fi
    done <<< "$loop_data"

    # If we found a loop point, trim the file
    if [ -n "$best_loop_start" ] && [ -n "$best_loop_end" ]; then
        if [ "$has_exclamation" = false ]; then
            echo "  循环: $best_loop_start → $best_loop_end (命令数: $best_loop_duration)"
        fi

        # Run vgm_trim
        ./vgm_trim.exe "$vgm_file" 0 "$best_loop_start" "$best_loop_end" "$output_file" > /dev/null 2>&1

        if [ -f "$output_file" ]; then
            # Get file sizes
            original_size=$(stat -c%s "$vgm_file" 2>/dev/null || stat -f%z "$vgm_file" 2>/dev/null)
            trimmed_size=$(stat -c%s "$output_file" 2>/dev/null || stat -f%z "$output_file" 2>/dev/null)

            if [ -n "$original_size" ] && [ -n "$trimmed_size" ]; then
                reduction=$((100 - (trimmed_size * 100 / original_size)))
                echo "  大小: $(numfmt --to=iec-i --suffix=B $original_size 2>/dev/null || echo "$original_size bytes") → $(numfmt --to=iec-i --suffix=B $trimmed_size 2>/dev/null || echo "$trimmed_size bytes") (减少${reduction}%)"
            fi

            trimmed_count=$((trimmed_count + 1))
        else
            echo "  错误: 修剪失败，复制原文件"
            cp "$vgm_file" "$output_file"
            copied_count=$((copied_count + 1))
        fi
    else
        echo "  复制: 无有效循环点"
        cp "$vgm_file" "$output_file"
        copied_count=$((copied_count + 1))
    fi

    echo ""
done

echo ""
echo "✓ 智能修剪完成"
echo "  修剪文件: $trimmed_count"
echo "  原样保留: $copied_count"
echo ""

# 步骤3: 清理临时文件
echo "========================================"
echo "步骤 3/3: 清理临时文件"
echo "========================================"
echo ""

echo "清理临时目录..."
rm -rf "$TEMP_DIR"

# 最终统计
total_files=$((trimmed_count + copied_count))

echo ""
echo "========================================"
echo "  完成！"
echo "========================================"
echo ""
echo "输出目录: $OUTPUT_DIR"
echo ""
echo "统计信息:"
echo "  总文件数: $total_files"
echo "  修剪文件: $trimmed_count (找到循环点)"
echo "  原样保留: $copied_count (无循环点)"
echo ""

# 计算总大小
if command -v du &> /dev/null; then
    total_size=$(du -sh "$OUTPUT_DIR" 2>/dev/null | cut -f1)
    echo "总大小: $total_size"
    echo ""
fi

echo "所有VGM文件已准备就绪，可以直接播放！"
echo "修剪后的文件已设置循环点，播放器会正确循环。"
echo ""
