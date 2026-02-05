#!/bin/bash
# GBS2VGM Complete Workflow - 完整自动化流程
# 功能: GBS转换 → 循环点检测 → VGM修剪 → 一键完成
#
# 使用方法:
#   ./gbs2vgm_complete.sh <m3u文件> <输出目录>
#
# 示例:
#   ./gbs2vgm_complete.sh "Dragon Quest I & II.m3u" "DQ12_Final"

set -e  # 遇到错误立即退出

# 检查参数
if [ $# -lt 2 ]; then
    echo "用法: $0 <M3U文件> <输出目录>"
    echo ""
    echo "示例:"
    echo "  $0 \"Dragon Quest I & II.m3u\" \"DQ12_Final\""
    exit 1
fi

M3U_FILE="$1"
OUTPUT_DIR="$2"
TEMP_DIR="${OUTPUT_DIR}_temp"

echo "========================================"
echo "  GBS2VGM 完整自动化流程 v2.3"
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

# 步骤2: 循环点检测
echo "========================================"
echo "步骤 2/3: 循环点检测"
echo "========================================"
echo ""

LOOP_REPORT="$TEMP_DIR/loop_report.txt"

# 创建循环点报告
echo "# Dragon Quest I & II - 循环点检测报告" > "$LOOP_REPORT"
echo "" >> "$LOOP_REPORT"

# 统计
total_files=0
loop_found=0

# 检测每个VGM文件的循环点
for vgm_file in "$TEMP_DIR"/*.vgm; do
    if [ -f "$vgm_file" ]; then
        filename=$(basename "$vgm_file")
        total_files=$((total_files + 1))

        echo "检测: $filename"

        # 运行vgmlpfnd
        echo "## $filename" >> "$LOOP_REPORT"

        result=$(echo "$vgm_file" | ./gbsplay/vgmlpfnd.exe -silent 2>/dev/null | grep "!" || echo "")

        if [ -n "$result" ]; then
            echo "$result" >> "$LOOP_REPORT"
            loop_found=$((loop_found + 1))
            echo "  ✓ 找到循环点"
        else
            echo "无循环点" >> "$LOOP_REPORT"
            echo "  - 无循环点"
        fi

        echo "" >> "$LOOP_REPORT"
    fi
done

echo ""
echo "✓ 循环点检测完成"
echo "  总文件数: $total_files"
echo "  找到循环点: $loop_found"
echo ""

# 步骤3: VGM修剪
echo "========================================"
echo "步骤 3/3: VGM文件修剪"
echo "========================================"
echo ""

# 创建最终输出目录
mkdir -p "$OUTPUT_DIR"

# 解析循环点报告并修剪文件
trimmed_count=0
copied_count=0
current_file=""
best_loop_start=""
best_loop_end=""
has_exclamation=false

while IFS= read -r line; do
    # 检查是否是文件头
    if [[ $line =~ ^##\ (.+\.vgm)$ ]]; then
        # 处理上一个文件
        if [ -n "$current_file" ]; then
            if [ -n "$best_loop_start" ] && [ -n "$best_loop_end" ]; then
                # 有循环点，执行修剪
                input_file="$TEMP_DIR/$current_file"
                output_file="$OUTPUT_DIR/$current_file"

                if [ -f "$input_file" ]; then
                    echo "修剪: $current_file"
                    if [ "$has_exclamation" = true ]; then
                        echo "  循环: $best_loop_start → $best_loop_end (最佳)"
                    else
                        echo "  循环: $best_loop_start → $best_loop_end"
                    fi

                    # 运行vgm_trim
                    ./vgm_trim.exe "$input_file" 0 "$best_loop_start" "$best_loop_end" "$output_file" > /dev/null 2>&1

                    if [ -f "$output_file" ]; then
                        # 获取文件大小
                        original_size=$(stat -c%s "$input_file" 2>/dev/null || stat -f%z "$input_file" 2>/dev/null)
                        trimmed_size=$(stat -c%s "$output_file" 2>/dev/null || stat -f%z "$output_file" 2>/dev/null)

                        if [ -n "$original_size" ] && [ -n "$trimmed_size" ]; then
                            reduction=$((100 - (trimmed_size * 100 / original_size)))
                            echo "  大小: $(numfmt --to=iec-i --suffix=B $original_size 2>/dev/null || echo "$original_size bytes") → $(numfmt --to=iec-i --suffix=B $trimmed_size 2>/dev/null || echo "$trimmed_size bytes") (减少${reduction}%)"
                        fi

                        trimmed_count=$((trimmed_count + 1))
                    else
                        echo "  错误: 修剪失败，复制原文件"
                        cp "$input_file" "$output_file"
                        copied_count=$((copied_count + 1))
                    fi
                fi
            fi
        fi

        # 开始新文件
        current_file="${BASH_REMATCH[1]}"
        best_loop_start=""
        best_loop_end=""
        has_exclamation=false
        continue
    fi

    # 检查是否是"无循环点"
    if [[ $line == "无循环点" ]]; then
        if [ -n "$current_file" ]; then
            # 直接复制无循环点的文件
            input_file="$TEMP_DIR/$current_file"
            output_file="$OUTPUT_DIR/$current_file"

            if [ -f "$input_file" ]; then
                cp "$input_file" "$output_file"
                echo "复制: $current_file (无循环点)"
                copied_count=$((copied_count + 1))
            fi
            current_file=""
            best_loop_start=""
            best_loop_end=""
            has_exclamation=false
        fi
        continue
    fi

    # 检查是否是带"!"的循环点行（最佳循环点）
    if [[ $line =~ ^([0-9]+)[[:space:]]+[0-9:.]+[[:space:]]+![[:space:]]+([0-9]+) ]]; then
        loop_start="${BASH_REMATCH[1]}"
        loop_end="${BASH_REMATCH[2]}"

        # 带"!"的循环点优先级最高，直接覆盖
        best_loop_start="$loop_start"
        best_loop_end="$loop_end"
        has_exclamation=true
        continue
    fi

    # 检查是否是普通循环点行（没有"!"）
    if [[ $line =~ ^([0-9]+)[[:space:]]+[0-9:.]+[[:space:]]+([0-9]+)[[:space:]]+[0-9:.]+[[:space:]]+[0-9]+ ]]; then
        loop_start="${BASH_REMATCH[1]}"
        loop_end="${BASH_REMATCH[2]}"

        # 如果还没有找到循环点，或者当前没有"!"标记的循环点，则使用这个
        if [ -z "$best_loop_start" ] && [ "$has_exclamation" = false ]; then
            best_loop_start="$loop_start"
            best_loop_end="$loop_end"
        fi
        continue
    fi
done < "$LOOP_REPORT"

# 处理最后一个文件
if [ -n "$current_file" ]; then
    if [ -n "$best_loop_start" ] && [ -n "$best_loop_end" ]; then
        # 有循环点，执行修剪
        input_file="$TEMP_DIR/$current_file"
        output_file="$OUTPUT_DIR/$current_file"

        if [ -f "$input_file" ]; then
            echo "修剪: $current_file"
            if [ "$has_exclamation" = true ]; then
                echo "  循环: $best_loop_start → $best_loop_end (最佳)"
            else
                echo "  循环: $best_loop_start → $best_loop_end"
            fi

            # 运行vgm_trim
            ./vgm_trim.exe "$input_file" 0 "$best_loop_start" "$best_loop_end" "$output_file" > /dev/null 2>&1

            if [ -f "$output_file" ]; then
                # 获取文件大小
                original_size=$(stat -c%s "$input_file" 2>/dev/null || stat -f%z "$input_file" 2>/dev/null)
                trimmed_size=$(stat -c%s "$output_file" 2>/dev/null || stat -f%z "$output_file" 2>/dev/null)

                if [ -n "$original_size" ] && [ -n "$trimmed_size" ]; then
                    reduction=$((100 - (trimmed_size * 100 / original_size)))
                    echo "  大小: $(numfmt --to=iec-i --suffix=B $original_size 2>/dev/null || echo "$original_size bytes") → $(numfmt --to=iec-i --suffix=B $trimmed_size 2>/dev/null || echo "$trimmed_size bytes") (减少${reduction}%)"
                fi

                trimmed_count=$((trimmed_count + 1))
            else
                echo "  错误: 修剪失败，复制原文件"
                cp "$input_file" "$output_file"
                copied_count=$((copied_count + 1))
            fi
        fi
    fi
fi

# 复制循环点报告到最终目录
cp "$LOOP_REPORT" "$OUTPUT_DIR/"

echo ""
echo "✓ VGM修剪完成"
echo "  修剪文件: $trimmed_count"
echo "  复制文件: $copied_count"
echo ""

# 清理临时目录
echo "清理临时文件..."
rm -rf "$TEMP_DIR"

# 最终统计
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
echo ""
