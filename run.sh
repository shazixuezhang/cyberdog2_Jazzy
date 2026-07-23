source /opt/ros/jazzy/setup.bash
source install/setup.bash
colcon build --merge-install --symlink-install --packages-up-to slam_toolbox \
  --cmake-args -DEIGEN3_INCLUDE_DIR=/usr/include/eigen3
colcon build --merge-install --symlink-install --packages-up-to \
    cyberdog_locomotion \
    cyberdog_simulator \
    motion_manager \
    params \
    cyberdog_web_controller \
    cyberdog_nav2_bringup &&
source install/setup.bash && 
python3 src/cyberdog_simulator/cyberdog_gazebo/script/launchsim.py
