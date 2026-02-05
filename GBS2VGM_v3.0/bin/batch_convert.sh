#!/bin/bash
# GBS2VGM 批量转换脚本
# 转换所有7z音乐包到VGM格式

echo "=== GBS2VGM 批量转换开始 ==="
echo "时间: $(date)"
echo ""

# 转换器路径
CONVERTER="./gbsplay/gbs2vgm_batch"

# 检查转换器是否存在
if [ ! -f "$CONVERTER" ]; then
    echo "错误: 找不到转换器 $CONVERTER"
    exit 1
fi

# 创建日志目录
LOG_DIR="conversion_logs"
mkdir -p "$LOG_DIR"

# 获取所有7z文件
mapfile -t FILES < <(ls *.7z 2>/dev/null)

if [ ${#FILES[@]} -eq 0 ]; then
    echo "错误: 未找到7z文件"
    exit 1
fi

echo "找到 ${#FILES[@]} 个7z文件"
echo ""

# 转换计数器
SUCCESS=0
FAILED=0
TOTAL=${#FILES[@]}

# 遍历所有7z文件
for FILE in "${FILES[@]}"; do
    echo "----------------------------------------"
    echo "[$((SUCCESS + FAILED + 1))/$TOTAL] 正在转换: $FILE"
    echo "开始时间: $(date '+%H:%M:%S')"

    # 生成日志文件名
    LOG_FILE="$LOG_DIR/$(basename "$FILE" .7z).log"

    # 执行转换
    if "$CONVERTER" "$FILE" > "$LOG_FILE" 2>&1; then
        echo "✓ 转换成功"
        SUCCESS=$((SUCCESS + 1))
    else
        echo "✗ 转换失败 (查看日志: $LOG_FILE)"
        FAILED=$((FAILED + 1))
    fi

    echo "结束时间: $(date '+%H:%M:%S')"
    echo ""
done

echo "========================================"
echo "批量转换完成！"
echo "总计: $TOTAL 个文件"
echo "成功: $SUCCESS 个"
echo "失败: $FAILED 个"
echo "日志目录: $LOG_DIR"
echo "完成时间: $(date)"
echo "========================================"

# 显示生成的VGM目录
echo ""
echo "生成的VGM目录:"
ls -d *_vgm*/ 2>/dev/null | while read dir; do
    count=$(ls "$dir"/*.vgm 2>/dev/null | wc -l)
    echo "  $dir - $count 个VGM文件"
done
