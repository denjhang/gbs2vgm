#!/bin/bash
# 重新生成循环点报告，保存完整的vgmlpfnd输出

INPUT_DIR="DQ12_VGM_v2.2"
LOOP_REPORT="$INPUT_DIR/loop_report_full.txt"

echo "# Dragon Quest I & II - 循环点检测报告（完整版）" > "$LOOP_REPORT"
echo "" >> "$LOOP_REPORT"

for vgm_file in "$INPUT_DIR"/*.vgm; do
    if [ -f "$vgm_file" ]; then
        filename=$(basename "$vgm_file")
        echo "检测: $filename"
        
        echo "## $filename" >> "$LOOP_REPORT"
        
        result=$(echo "$vgm_file" | ./gbsplay/vgmlpfnd.exe -silent 2>/dev/null)
        
        if [ -n "$result" ]; then
            echo "$result" >> "$LOOP_REPORT"
        else
            echo "无循环点" >> "$LOOP_REPORT"
        fi
        
        echo "" >> "$LOOP_REPORT"
    fi
done

echo "完成！报告保存到: $LOOP_REPORT"
