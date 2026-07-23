#!/bin/bash

source /opt/ros/jazzy/setup.bash
source install/setup.bash

# 终端 2：启动 Nav2（边建图边导航）
ros2 launch cyberdog_nav2_bringup cyberdog_navigation.launch.py