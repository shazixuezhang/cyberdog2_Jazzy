source /opt/ros/jazzy/setup.bash
rm -rf build/ log/ install/
# colcon build --merge-install --symlink-install --packages-up-to slam_toolbox \
#   --cmake-args -DEIGEN3_INCLUDE_DIR=/usr/include/eigen3
colcon build --merge-install --symlink-install --packages-up-to \
    cyberdog_locomotion \
    cyberdog_simulator \
    cyberdog_motion_manager \
    cyberdog_params \
    cyberdog_web_controller 
if [ -f install/setup.bash ]; then
    source install/setup.bash && 
    python3 src/cyberdog_simulator/cyberdog_gazebo/script/launchsim.py
fi
