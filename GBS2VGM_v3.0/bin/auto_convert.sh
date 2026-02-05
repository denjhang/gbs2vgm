#!/bin/bash
# GBS2VGM 自动循环检测和修剪脚本

if [ $# -lt 2 ]; then
    echo "用法: $0 <M3U文件> <输出目录>"
    exit 1
fi

M3U_FILE="$1"
OUTPUT_DIR="$2"

echo "=== GBS2VGM 完整转换流程 ==="
echo "M3U文件: $M3U_FILE"
echo "输出目录: $OUTPUT_DIR"
echo ""

# 步骤1: 转换GBS到VGM (3倍长度用于循环检测)
echo "步骤1: 转换GBS到VGM..."
./gbs2vgm_batch.exe "$M3U_FILE" "$OUTPUT_DIR"
if [ $? -ne 0 ]; then
    echo "错误: GBS转换失败"
    exit 1
fi

# 步骤2: 对每个VGM文件进行循环检测和修剪
echo ""
echo "步骤2: 循环检测和修剪..."
cd "$OUTPUT_DIR"

for vgm in *.vgm; do
    if [ ! -f "$vgm" ]; then
        continue
    fi

    echo "  处理: $vgm"

    # 运行vgmlpfnd检测循环点
    loop_info=$(echo "$vgm" | ../gbsplay/vgmlpfnd.exe -silent 2>&1 | grep "!" | head -1)

    if [ -n "$loop_info" ]; then
        echo "    找到循环点: $loop_info"
        # TODO: 解析循环点并修剪VGM文件
        # 这里需要一个VGM修剪工具
    else
        echo "    未找到循环点 (非循环曲目)"
    fi
done

cd ..

echo ""
echo "=== 转换完成 ==="
