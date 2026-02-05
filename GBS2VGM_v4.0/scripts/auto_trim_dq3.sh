#!/bin/bash
# Smart VGM Auto-Trim Script for DQ3
# 智能选择循环点：优先"!"，其次选择持续时间最长的循环

INPUT_DIR="working/DQ3_VGM"
OUTPUT_DIR="working/DQ3_VGM_trimmed"

# Create output directory
mkdir -p "$OUTPUT_DIR"

echo "=== DQ3 智能VGM修剪脚本 ==="
echo "输入目录: $INPUT_DIR"
echo "输出目录: $OUTPUT_DIR"
echo ""

# Check if vgm_trim.exe exists
if [ ! -f "bin/vgm_trim.exe" ]; then
    echo "错误: bin/vgm_trim.exe 不存在"
    exit 1
fi

trimmed_count=0
skipped_count=0

# Process each VGM file
for vgm_file in "$INPUT_DIR"/*.vgm; do
    if [ ! -f "$vgm_file" ]; then
        continue
    fi

    # Skip already trimmed files
    if [[ "$vgm_file" == *"_trimmed"* ]]; then
        continue
    fi

    filename=$(basename "$vgm_file")
    output_file="$OUTPUT_DIR/$filename"

    echo "处理: $filename"

    # Run vgmlpfnd and capture output
    loop_data=$(echo "$vgm_file" | ./gbsplay/vgmlpfnd.exe -silent 2>/dev/null)

    if [ -z "$loop_data" ]; then
        echo "  跳过: 无循环点"
        skipped_count=$((skipped_count + 1))
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
            echo "  找到最佳循环点: $loop_start -> $loop_end (命令数: $loop_cmds)"
            break
        fi

        # Check for loop point with "e" (end of file) or regular loop point
        if [[ $line =~ ^([0-9]+)[[:space:]]+[0-9:.]+[[:space:]]+(e)?[[:space:]]*([0-9]+)[[:space:]]+[0-9:.]+[[:space:]]+([0-9]+) ]]; then
            loop_start="${BASH_REMATCH[1]}"
            loop_end="${BASH_REMATCH[3]}"
            loop_cmds="${BASH_REMATCH[4]}"

            # Calculate loop duration (use command count as proxy)
            if [ "$loop_cmds" -gt "$best_loop_duration" ]; then
                best_loop_start="$loop_start"
                best_loop_end="$loop_end"
                best_loop_duration="$loop_cmds"
            fi
        fi
    done <<< "$loop_data"

    # If we found a loop point, trim the file
    if [ -n "$best_loop_start" ] && [ -n "$best_loop_end" ]; then
        if [ "$has_exclamation" = false ]; then
            echo "  使用最长循环点: $best_loop_start -> $best_loop_end (命令数: $best_loop_duration)"
        fi

        # Run vgm_trim
        ./bin/vgm_trim.exe "$vgm_file" 0 "$best_loop_start" "$best_loop_end" "$output_file" > /dev/null 2>&1

        if [ -f "$output_file" ]; then
            # Get file sizes
            original_size=$(stat -c%s "$vgm_file" 2>/dev/null || stat -f%z "$vgm_file" 2>/dev/null)
            trimmed_size=$(stat -c%s "$output_file" 2>/dev/null || stat -f%z "$output_file" 2>/dev/null)

            if [ -n "$original_size" ] && [ -n "$trimmed_size" ]; then
                reduction=$((100 - (trimmed_size * 100 / original_size)))
                echo "  大小: $(numfmt --to=iec-i --suffix=B $original_size 2>/dev/null || echo "$original_size bytes") -> $(numfmt --to=iec-i --suffix=B $trimmed_size 2>/dev/null || echo "$trimmed_size bytes") (减少${reduction}%)"
            fi

            trimmed_count=$((trimmed_count + 1))
        else
            echo "  错误: 修剪失败"
        fi
    else
        echo "  跳过: 无有效循环点"
        skipped_count=$((skipped_count + 1))
    fi

    echo ""
done

echo "=== 完成 ==="
echo "修剪文件: $trimmed_count"
echo "跳过文件: $skipped_count"
echo "输出目录: $OUTPUT_DIR"
echo ""
