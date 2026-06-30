#!/bin/bash

source /opt/ros/jazzy/setup.bash
source install/setup.bash
export GZ_SIM_RESOURCE_PATH=/home/lq/.gz/models/osrf_base_models:/opt/ros/jazzy/share
chmod +x src/cyberdog_sim/cyberdog_simulator/cyberdog_gazebo/script/gazebolauncher.py

# ros2 launch cyberdog_gazebo gazebo.launch.py
python3 src/cyberdog_sim/cyberdog_simulator/cyberdog_gazebo/script/gazebolauncher.py ros2 launch cyberdog_gazebo gazebo.launch.py  use_lidar:=true