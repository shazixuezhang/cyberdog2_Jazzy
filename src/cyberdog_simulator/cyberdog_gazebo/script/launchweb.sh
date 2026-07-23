#!/bin/bash

source /opt/ros/jazzy/setup.bash
source install/setup.bash

source ~/cyberdog_venv/bin/activate
ros2 launch cyberdog_web_controller web_controller.launch.py