#!/bin/bash
# 多线程VGM处理脚本 - 边检测边修剪，生成详细日志
# 用法: ./process_vgm_parallel.sh <文件夹名称> [线程数]
# 例如: ./process_vgm_parallel.sh DQ3 8

if [ -z "$1" ]; then
    echo "用法: $0 <文件夹名称> [线程数]"
    echo "例如: $0 DQ3 8"
    echo "      $0 DQM1 4"
    echo "      $0 DQM2"
    echo "      $0 Tales 6"
    exit 1
fi

FOLDER_NAME=$1
THREADS=${2:-4}  # 默认4线程
INPUT_DIR="working/${FOLDER_NAME}_VGM"
TEMP_OUTPUT_DIR="working/${FOLDER_NAME}_VGM_trimmed_temp"
LOG_FILE="working/${FOLDER_NAME}_VGM_process.log"
PROGRESS_FILE="working/${FOLDER_NAME}_VGM_progress.txt"
LOCK_FILE="working/${FOLDER_NAME}_VGM.lock"

# 初始化日志
echo "=========================================="  | tee "$LOG_FILE"
echo "=== ${FOLDER_NAME} VGM多线程处理脚本 ==="  | tee -a "$LOG_FILE"
echo "=========================================="  | tee -a "$LOG_FILE"
echo "开始时间: $(date '+%Y-%m-%d %H:%M:%S')"  | tee -a "$LOG_FILE"
echo "输入目录: $INPUT_DIR"  | tee -a "$LOG_FILE"
echo "线程数: $THREADS"  | tee -a "$LOG_FILE"
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

# 初始化计数器文件
echo "0" > "${TEMP_OUTPUT_DIR}/trimmed_count.txt"
echo "0" > "${TEMP_OUTPUT_DIR}/skipped_count.txt"
echo "0" > "${TEMP_OUTPUT_DIR}/error_count.txt"
echo "0" > "${TEMP_OUTPUT_DIR}/processed_count.txt"

# 统计总文件数
total_files=$(ls -1 "$INPUT_DIR"/*.vgm 2>/dev/null | wc -l)
echo "总文件数: $total_files"  | tee -a "$LOG_FILE"
echo ""  | tee -a "$LOG_FILE"

# 处理单个文件的函数
process_file() {
    local vgm_file=$1
    local file_num=$2
    local total=$3
    local temp_dir=$4
    local log_file=$5

    filename=$(basename "$vgm_file")
    output_file="$temp_dir/$filename"

    # 创建临时日志
    temp_log="${temp_dir}/${filename}.log"

    echo "----------------------------------------" > "$temp_log"
    echo "[$file_num/$total] 处理: $filename" >> "$temp_log"
    echo "时间: $(date '+%H:%M:%S')" >> "$temp_log"

    # 运行vgmlpfnd并捕获输出
    loop_data=$(echo "$vgm_file" | ./gbsplay/vgmlpfnd.exe -silent 2>/dev/null)

    if [ -z "$loop_data" ]; then
        echo "  结果: 跳过 - 无循环点" >> "$temp_log"
        echo "" >> "$temp_log"

        # 原子性增加跳过计数
        flock "${temp_dir}/skipped_count.txt" bash -c "echo \$(($(cat ${temp_dir}/skipped_count.txt) + 1)) > ${temp_dir}/skipped_count.txt"
        flock "${temp_dir}/processed_count.txt" bash -c "echo \$(($(cat ${temp_dir}/processed_count.txt) + 1)) > ${temp_dir}/processed_count.txt"

        # 追加到主日志
        flock "$log_file" cat "$temp_log" >> "$log_file"
        rm "$temp_log"
        return
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
            echo "  循环点: $loop_start -> $loop_end (命令数: $loop_cmds) [最佳!]" >> "$temp_log"
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
            echo "  循环点: $best_loop_start -> $best_loop_end (命令数: $best_loop_duration) [最长]" >> "$temp_log"
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
                echo "  大小: ${original_kb}KB -> ${trimmed_kb}KB (减少${reduction}%)" >> "$temp_log"
            fi

            echo "  结果: 修剪成功" >> "$temp_log"

            # 原子性增加修剪计数
            flock "${temp_dir}/trimmed_count.txt" bash -c "echo \$(($(cat ${temp_dir}/trimmed_count.txt) + 1)) > ${temp_dir}/trimmed_count.txt"
        else
            echo "  结果: 修剪失败" >> "$temp_log"

            # 原子性增加错误计数
            flock "${temp_dir}/error_count.txt" bash -c "echo \$(($(cat ${temp_dir}/error_count.txt) + 1)) > ${temp_dir}/error_count.txt"
        fi
    else
        echo "  结果: 跳过 - 无有效循环点" >> "$temp_log"

        # 原子性增加跳过计数
        flock "${temp_dir}/skipped_count.txt" bash -c "echo \$(($(cat ${temp_dir}/skipped_count.txt) + 1)) > ${temp_dir}/skipped_count.txt"
    fi

    echo "" >> "$temp_log"

    # 原子性增加已处理计数
    flock "${temp_dir}/processed_count.txt" bash -c "echo \$(($(cat ${temp_dir}/processed_count.txt) + 1)) > ${temp_dir}/processed_count.txt"

    # 追加到主日志
    flock "$log_file" cat "$temp_log" >> "$log_file"
    rm "$temp_log"
}

# 导出函数和变量供子进程使用
export -f process_file
export TEMP_OUTPUT_DIR
export LOG_FILE

# 创建文件列表
file_list=()
for vgm_file in "$INPUT_DIR"/*.vgm; do
    if [ -f "$vgm_file" ] && [[ "$vgm_file" != *"_trimmed"* ]]; then
        file_list+=("$vgm_file")
    fi
done

# 启动进度监控（后台）
(
    while true; do
        if [ -f "${TEMP_OUTPUT_DIR}/processed_count.txt" ]; then
            processed=$(cat "${TEMP_OUTPUT_DIR}/processed_count.txt" 2>/dev/null || echo "0")
            trimmed=$(cat "${TEMP_OUTPUT_DIR}/trimmed_count.txt" 2>/dev/null || echo "0")
            skipped=$(cat "${TEMP_OUTPUT_DIR}/skipped_count.txt" 2>/dev/null || echo "0")
            errors=$(cat "${TEMP_OUTPUT_DIR}/error_count.txt" 2>/dev/null || echo "0")

            echo "进度: $processed / $total_files" > "$PROGRESS_FILE"
            echo "已修剪: $trimmed" >> "$PROGRESS_FILE"
            echo "已跳过: $skipped" >> "$PROGRESS_FILE"
            echo "错误: $errors" >> "$PROGRESS_FILE"
            echo "更新时间: $(date '+%H:%M:%S')" >> "$PROGRESS_FILE"

            if [ "$processed" -ge "$total_files" ]; then
                break
            fi
        fi
        sleep 2
    done
) &
progress_monitor_pid=$!

# 并行处理文件
echo "开始并行处理 (${THREADS}线程)..."
file_num=0
for vgm_file in "${file_list[@]}"; do
    file_num=$((file_num + 1))

    # 等待直到有可用的线程槽
    while [ $(jobs -r | wc -l) -ge $THREADS ]; do
        sleep 0.1
    done

    # 在后台处理文件
    process_file "$vgm_file" "$file_num" "$total_files" "$TEMP_OUTPUT_DIR" "$LOG_FILE" &
done

# 等待所有后台任务完成
wait

# 停止进度监控
kill $progress_monitor_pid 2>/dev/null
wait $progress_monitor_pid 2>/dev/null

# 读取最终计数
trimmed_count=$(cat "${TEMP_OUTPUT_DIR}/trimmed_count.txt")
skipped_count=$(cat "${TEMP_OUTPUT_DIR}/skipped_count.txt")
error_count=$(cat "${TEMP_OUTPUT_DIR}/error_count.txt")

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
