#!/bin/bash
# 监控转换进度

while true; do
    clear
    echo "=== GBS2VGM 转换进度监控 ==="
    echo "时间: $(date '+%H:%M:%S')"
    echo ""
    
    # 检查进程
    if ps aux | grep -q "[g]bs2vgm_batch"; then
        echo "状态: ✓ 转换进行中"
    else
        echo "状态: ✗ 转换已完成或未运行"
    fi
    echo ""
    
    # 显示已生成的VGM目录
    echo "已生成的VGM目录:"
    for dir in *_vgm*/; do
        if [ -d "$dir" ]; then
            count=$(ls "$dir"/*.vgm 2>/dev/null | wc -l)
            size=$(du -sh "$dir" 2>/dev/null | cut -f1)
            echo "  $dir"
            echo "    文件数: $count"
            echo "    大小: $size"
        fi
    done
    echo ""
    
    # 显示最新日志
    if [ -d "conversion_logs" ]; then
        echo "最新日志 (最后10行):"
        latest_log=$(ls -t conversion_logs/*.log 2>/dev/null | head -1)
        if [ -n "$latest_log" ]; then
            echo "  文件: $(basename "$latest_log")"
            tail -10 "$latest_log" 2>/dev/null | sed 's/^/    /'
        fi
    fi
    
    sleep 5
done
