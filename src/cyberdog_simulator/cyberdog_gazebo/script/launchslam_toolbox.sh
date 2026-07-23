#!/bin/bash

source /opt/ros/jazzy/setup.bash
source install/setup.bash

# 尝试启动异步建图
ros2 launch slam_toolbox online_async_launch.py use_sim_time:=true