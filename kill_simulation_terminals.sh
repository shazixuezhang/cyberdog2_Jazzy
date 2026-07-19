#!/bin/bash
# 终止铁蛋仿真3个gnome-terminal终端进程
echo "正在查找并关闭 cyberdog_gazebo / cyberdog_control / cyberdog_visual 终端..."

# 通过窗口标题匹配并杀死对应终端进程
pkill -f "gnome-terminal.*cyberdog_gazebo"
pkill -f "gnome-terminal.*cyberdog_control"
pkill -f "gnome-terminal.*cyberdog_viusal"

# 额外清理残留gz-sim仿真进程（可选）
# pkill -f gz sim

echo "✅ 仿真终端已关闭"
