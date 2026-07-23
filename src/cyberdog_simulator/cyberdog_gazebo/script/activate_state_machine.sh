#!/bin/bash

source /opt/ros/jazzy/setup.bash
source install/setup.bash

ros2 service  call /motion_managermachine_service  protocol/srv/FsMachine "target_state: 'Active'"