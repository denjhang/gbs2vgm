#!/bin/bash
# 完整的DQ1转换流程

echo "=== Dragon Quest I & II 完整转换流程 ==="
echo ""

# 步骤1: 转换
echo "步骤1: 转换GBS到VGM (3倍长度)..."
./gbs2vgm_batch.exe "Dragon Warrior I & II [Dragon Quest I+II] (1999-09-23)(Chunsoft)(Tose)(Enix)[GBC]/DMG-AEDJ-JPN.m3u" "DQ12_VGM_v2.2"

if [ $? -ne 0 ]; then
    echo "错误: 转换失败"
    exit 1
fi

echo ""
echo "步骤2: 循环点检测..."
cd "DQ12_VGM_v2.2"

# 创建循环点报告
echo "# Dragon Quest I & II - 循环点检测报告" > loop_report.txt
echo "" >> loop_report.txt

for vgm in *.vgm; do
    if [ ! -f "$vgm" ]; then
        continue
    fi

    echo "  检测: $vgm"
    echo "## $vgm" >> loop_report.txt

    # 运行vgmlpfnd
    loop_output=$(echo "$vgm" | ../gbsplay/vgmlpfnd.exe -silent 2>&1)

    # 查找最佳循环点 (标记为!)
    best_loop=$(echo "$loop_output" | grep "!" | head -1)

    if [ -n "$best_loop" ]; then
        echo "    找到循环点: $best_loop"
        echo "$best_loop" >> loop_report.txt
    else
        echo "    未找到循环点 (非循环曲目)"
        echo "无循环点" >> loop_report.txt
    fi

    echo "" >> loop_report.txt
done

cd ..

echo ""
echo "=== 转换完成 ==="
echo "输出目录: DQ12_VGM_v2.2"
echo "循环点报告: DQ12_VGM_v2.2/loop_report.txt"
echo ""
echo "注意: VGM文件包含3倍长度的数据用于循环检测"
echo "      循环点信息已保存到loop_report.txt"
echo "      后续可以使用这些信息修剪VGM文件"
