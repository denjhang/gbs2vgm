#!/bin/bash
# 带日志的VGM处理脚本 - 边检测边修剪，生成详细日志
# 用法: ./process_vgm_with_log.sh <文件夹名称>
# 例如: ./process_vgm_with_log.sh DQ3

if [ -z "$1" ]; then
    echo "用法: $0 <文件夹名称>"
    echo "例如: $0 DQ3"
    echo "      $0 DQM1"
    echo "      $0 DQM2"
    echo "      $0 Tales"
    exit 1
fi

FOLDER_NAME=$1
INPUT_DIR="working/${FOLDER_NAME}_VGM"
TEMP_OUTPUT_DIR="working/${FOLDER_NAME}_VGM_trimmed_temp"
LOG_FILE="working/${FOLDER_NAME}_VGM_process.log"
PROGRESS_FILE="working/${FOLDER_NAME}_VGM_progress.txt"

# 初始化日志
echo "=========================================="  | tee "$LOG_FILE"
echo "=== ${FOLDER_NAME} VGM处理脚本 ==="  | tee -a "$LOG_FILE"
echo "=========================================="  | tee -a "$LOG_FILE"
echo "开始时间: $(date '+%Y-%m-%d %H:%M:%S')"  | tee -a "$LOG_FILE"
echo "输入目录: $INPUT_DIR"  | tee -a "$LOG_FILE"
echo "日志文件: $LOG_FILE"  | tee -a "$LOG_FILE"
echo "进度文件: $PROGRESS_FILE"  | tee -a "$LOG_FILE"
echo ""  | tee -a "$LOG_FILE"

# 检查输入目录
if [ ! -d "$INPUT_DIR" ]; then
    echo "错误: 输入目录不存在: $INPUT_DIR"  | tee -a "$LOG_FILE"
    exit 1
fi

# 检查工具
if [ ! -f "bin/vgm_trim.exe" ]; then
    echo "错误: bin/vgm_trim.exe 不存在"  | tee -a "$LOG_FILE"
    exit 1
fi

if [ ! -f "gbsplay/vgmlpfnd.exe" ]; then
    echo "错误: gbsplay/vgmlpfnd.exe 不存在"  | tee -a "$LOG_FILE"
    exit 1
fi

# 创建临时输出目录
mkdir -p "$TEMP_OUTPUT_DIR"

# 统计总文件数
total_files=$(ls -1 "$INPUT_DIR"/*.vgm 2>/dev/null | wc -l)
echo "总文件数: $total_files"  | tee -a "$LOG_FILE"
echo ""  | tee -a "$LOG_FILE"

trimmed_count=0
skipped_count=0
error_count=0
current_file_num=0

# 处理每个VGM文件
for vgm_file in "$INPUT_DIR"/*.vgm; do
    if [ ! -f "$vgm_file" ]; then
        continue
    fi

    # 跳过已修剪的文件
    if [[ "$vgm_file" == *"_trimmed"* ]]; then
        continue
    fi

    current_file_num=$((current_file_num + 1))
    filename=$(basename "$vgm_file")
    output_file="$TEMP_OUTPUT_DIR/$filename"

    # 更新进度文件
    echo "进度: $current_file_num / $total_files" > "$PROGRESS_FILE"
    echo "当前文件: $filename" >> "$PROGRESS_FILE"
    echo "已修剪: $trimmed_count" >> "$PROGRESS_FILE"
    echo "已跳过: $skipped_count" >> "$PROGRESS_FILE"
    echo "错误: $error_count" >> "$PROGRESS_FILE"

    echo "----------------------------------------"  | tee -a "$LOG_FILE"
    echo "[$current_file_num/$total_files] 处理: $filename"  | tee -a "$LOG_FILE"
    echo "时间: $(date '+%H:%M:%S')"  | tee -a "$LOG_FILE"

    # 运行vgmlpfnd并捕获输出
    loop_data=$(echo "$vgm_file" | ./gbsplay/vgmlpfnd.exe -silent 2>/dev/null)

    if [ -z "$loop_data" ]; then
        echo "  结果: 跳过 - 无循环点"  | tee -a "$LOG_FILE"
        skipped_count=$((skipped_count + 1))
        echo ""  | tee -a "$LOG_FILE"
        continue
    fi

    # 解析循环数据并找到最佳循环点
    best_loop_start=""
    best_loop_end=""
    best_loop_duration=0
    has_exclamation=false

    while IFS= read -r line; do
        # 跳过标题行
        if [[ $line =~ ^Source || $line =~ ^Start ]]; then
            continue
        fi

        # 检查带"!"的循环点（最高优先级）
        if [[ $line =~ ^([0-9]+)[[:space:]]+[0-9:.]+[[:space:]]+![[:space:]]+([0-9]+)[[:space:]]+[0-9:.]+[[:space:]]+([0-9]+) ]]; then
            loop_start="${BASH_REMATCH[1]}"
            loop_end="${BASH_REMATCH[2]}"
            loop_cmds="${BASH_REMATCH[3]}"

            best_loop_start="$loop_start"
            best_loop_end="$loop_end"
            has_exclamation=true
            echo "  循环点: $loop_start -> $loop_end (命令数: $loop_cmds) [最佳!]"  | tee -a "$LOG_FILE"
            break
        fi

        # 检查带"e"（文件结尾）或常规循环点
        if [[ $line =~ ^([0-9]+)[[:space:]]+[0-9:.]+[[:space:]]+(e)?[[:space:]]*([0-9]+)[[:space:]]+[0-9:.]+[[:space:]]+([0-9]+) ]]; then
            loop_start="${BASH_REMATCH[1]}"
            loop_end="${BASH_REMATCH[3]}"
            loop_cmds="${BASH_REMATCH[4]}"

            # 计算循环持续时间（使用命令数作为代理）
            if [ "$loop_cmds" -gt "$best_loop_duration" ]; then
                best_loop_start="$loop_start"
                best_loop_end="$loop_end"
                best_loop_duration="$loop_cmds"
            fi
        fi
    done <<< "$loop_data"

    # 如果找到循环点，修剪文件
    if [ -n "$best_loop_start" ] && [ -n "$best_loop_end" ]; then
        if [ "$has_exclamation" = false ]; then
            echo "  循环点: $best_loop_start -> $best_loop_end (命令数: $best_loop_duration) [最长]"  | tee -a "$LOG_FILE"
        fi

        # 运行vgm_trim
        ./bin/vgm_trim.exe "$vgm_file" 0 "$best_loop_start" "$best_loop_end" "$output_file" > /dev/null 2>&1

        if [ -f "$output_file" ]; then
            # 获取文件大小
            original_size=$(stat -c%s "$vgm_file" 2>/dev/null || stat -f%z "$vgm_file" 2>/dev/null)
            trimmed_size=$(stat -c%s "$output_file" 2>/dev/null || stat -f%z "$output_file" 2>/dev/null)

            if [ -n "$original_size" ] && [ -n "$trimmed_size" ]; then
                reduction=$((100 - (trimmed_size * 100 / original_size)))
                original_kb=$((original_size / 1024))
                trimmed_kb=$((trimmed_size / 1024))
                echo "  大小: ${original_kb}KB -> ${trimmed_kb}KB (减少${reduction}%)"  | tee -a "$LOG_FILE"
            fi

            echo "  结果: 修剪成功"  | tee -a "$LOG_FILE"
            trimmed_count=$((trimmed_count + 1))
        else
            echo "  结果: 修剪失败"  | tee -a "$LOG_FILE"
            error_count=$((error_count + 1))
        fi
    else
        echo "  结果: 跳过 - 无有效循环点"  | tee -a "$LOG_FILE"
        skipped_count=$((skipped_count + 1))
    fi

    echo ""  | tee -a "$LOG_FILE"
done

echo "=========================================="  | tee -a "$LOG_FILE"
echo "=== 修剪阶段完成 ==="  | tee -a "$LOG_FILE"
echo "=========================================="  | tee -a "$LOG_FILE"
echo "总文件数: $total_files"  | tee -a "$LOG_FILE"
echo "修剪成功: $trimmed_count"  | tee -a "$LOG_FILE"
echo "跳过文件: $skipped_count"  | tee -a "$LOG_FILE"
echo "错误: $error_count"  | tee -a "$LOG_FILE"
echo ""  | tee -a "$LOG_FILE"

# ========================================
# 合并文件到原目录
# ========================================
echo "=========================================="  | tee -a "$LOG_FILE"
echo "=== 合并文件到原目录 ==="  | tee -a "$LOG_FILE"
echo "=========================================="  | tee -a "$LOG_FILE"
echo ""  | tee -a "$LOG_FILE"

merged_count=0
kept_count=0

# 将修剪后的文件复制回原目录（覆盖）
for trimmed_file in "$TEMP_OUTPUT_DIR"/*.vgm; do
    if [ ! -f "$trimmed_file" ]; then
        continue
    fi

    filename=$(basename "$trimmed_file")
    cp "$trimmed_file" "$INPUT_DIR/$filename"
    echo "已覆盖: $filename"  | tee -a "$LOG_FILE"
    merged_count=$((merged_count + 1))
done

echo ""  | tee -a "$LOG_FILE"

# 统计未修剪的文件（保持原样）
for original_file in "$INPUT_DIR"/*.vgm; do
    if [ ! -f "$original_file" ]; then
        continue
    fi

    filename=$(basename "$original_file")

    # 检查是否有对应的修剪文件
    if [ ! -f "$TEMP_OUTPUT_DIR/$filename" ]; then
        echo "保留原文件: $filename"  | tee -a "$LOG_FILE"
        kept_count=$((kept_count + 1))
    fi
done

echo ""  | tee -a "$LOG_FILE"
echo "覆盖文件: $merged_count"  | tee -a "$LOG_FILE"
echo "保留原文件: $kept_count"  | tee -a "$LOG_FILE"
echo ""  | tee -a "$LOG_FILE"

# 清理临时目录
echo "清理临时目录..."  | tee -a "$LOG_FILE"
rm -rf "$TEMP_OUTPUT_DIR"

echo ""  | tee -a "$LOG_FILE"
echo "=========================================="  | tee -a "$LOG_FILE"
echo "=== 全部完成 ==="  | tee -a "$LOG_FILE"
echo "=========================================="  | tee -a "$LOG_FILE"
echo "结束时间: $(date '+%Y-%m-%d %H:%M:%S')"  | tee -a "$LOG_FILE"
echo "最终目录: $INPUT_DIR"  | tee -a "$LOG_FILE"
echo "日志文件: $LOG_FILE"  | tee -a "$LOG_FILE"
echo ""  | tee -a "$LOG_FILE"

# 更新最终进度
echo "状态: 完成" > "$PROGRESS_FILE"
echo "总文件数: $total_files" >> "$PROGRESS_FILE"
echo "修剪成功: $trimmed_count" >> "$PROGRESS_FILE"
echo "跳过文件: $skipped_count" >> "$PROGRESS_FILE"
echo "错误: $error_count" >> "$PROGRESS_FILE"
echo "覆盖文件: $merged_count" >> "$PROGRESS_FILE"
echo "保留原文件: $kept_count" >> "$PROGRESS_FILE"
