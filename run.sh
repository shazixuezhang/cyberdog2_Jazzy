colcon build --merge-install --symlink-install --packages-up-to cyberdog_locomotion cyberdog_simulator &&
source install/setup.bash && 
python3 src/cyberdog_sim/cyberdog_simulator/cyberdog_gazebo/script/launchsim.py