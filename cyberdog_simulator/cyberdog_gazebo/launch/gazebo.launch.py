#!/usr/bin/python3
#
# Copyright (c) 2023-2023 Beijing Xiaomi Mobile Software Co., Ltd. All rights reserved.

# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at

#      http://www.apache.org/licenses/LICENSE-2.0

# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os
import tempfile
import launch
from launch.conditions import IfCondition
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.actions import OpaqueFunction, TimerAction
from launch_ros.actions import Node
from ament_index_python.packages import get_package_prefix
from ament_index_python.packages import get_package_share_directory
import xacro


def launch_setup(context, *args, **kwargs):
    # pkg
    pkg_share = get_package_share_directory('cyberdog_gazebo')
    pkg_prefix = get_package_prefix('cyberdog_gazebo')
    pkg_ros_gz_sim = get_package_share_directory('ros_gz_sim')

    # config
    hang_robot = LaunchConfiguration('hang_robot').perform(context)
    use_lidar = LaunchConfiguration('use_lidar').perform(context)
    wname = LaunchConfiguration('wname').perform(context)
    rname = LaunchConfiguration('rname').perform(context)

    # env - gz-sim uses different environment variables than Gazebo Classic
    gz_model_path = os.path.join(pkg_share, 'model')
    gz_plugin_path = os.path.join(get_package_prefix('cyberdog_gazebo'), 'lib')
    existing_resource_path = os.environ.get("GZ_SIM_RESOURCE_PATH", "")
    existing_plugin_path = os.environ.get("GZ_SIM_SYSTEM_PLUGIN_PATH", "")
    os.environ["GZ_SIM_RESOURCE_PATH"] = gz_model_path + (os.pathsep + existing_resource_path if existing_resource_path else "")
    os.environ["GZ_SIM_SYSTEM_PLUGIN_PATH"] = gz_plugin_path + (os.pathsep + existing_plugin_path if existing_plugin_path else "")

    # world
    world_path = os.path.join(pkg_share, 'world', wname+'.world')

    # urdf
    xacro_path = os.path.join(get_package_share_directory(
        rname+'_description'), 'xacro', 'robot.xacro')
    urdf_contents = xacro.process_file(xacro_path, mappings={
                                       'DEBUG': hang_robot, 'USE_LIDAR': use_lidar}).toprettyxml(indent='  ')

    # Write URDF to a temp file for gz-sim create command
    with tempfile.NamedTemporaryFile(mode='w', suffix='.urdf', delete=False) as f:
        f.write(urdf_contents)
        urdf_file_path = f.name

    # spawn entity using gz-sim create command
    spawn_entity = launch.actions.ExecuteProcess(
        name='spawn_entity',
        cmd=['ros2', 'run', 'ros_gz_sim', 'create',
             '-world', 'earth',
             '-name', rname,
             '-x', '0', '-y', '0', '-z', '0.31',
             '-file', urdf_file_path],
        log_cmd=False)

    # Build gz_args string for gz-sim
    gz_args = world_path
    if LaunchConfiguration("paused").perform(context) != 'true':
        gz_args += ' -r'
    if LaunchConfiguration("headless").perform(context) == 'true':
        gz_args += ' -s'

    start_gz_sim = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_ros_gz_sim, "launch", "gz_sim.launch.py")
        ),
        condition=IfCondition(LaunchConfiguration("use_simulator")),
        launch_arguments={
            "gz_args": gz_args,
        }.items(),
    )

    # ROS2 <-> GZ bridge for sensors
    # Use ros_gz_bridge's automatic type mapping
    # Format: gz_topic[ros2_topic  (GZ -> ROS, auto-detect types)

    # IMU: gz topic -> ROS2 /imu
    imu_bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        name='imu_bridge',
        arguments=[
            '/imu' + '[' + '/imu'
        ],
        output='screen'
    )

    # Lidar: gz topic -> ROS2 /scan (conditional)
    scan_bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        name='scan_bridge',
        arguments=[
            '/scan' + '[' + '/scan'
        ],
        output='screen',
        condition=IfCondition(use_lidar)
    )

    # Delay spawn entity to ensure gz-sim world is ready
    delayed_spawn = TimerAction(
        period=3.0,
        actions=[spawn_entity]
    )

    return [
        start_gz_sim,
        imu_bridge,
        scan_bridge,
        delayed_spawn]


def generate_launch_description():
    ld = launch.LaunchDescription([
        DeclareLaunchArgument(
            name='headless',
            default_value='False',
            description='Whether to execute gzclient'
        ),
        DeclareLaunchArgument(
            name='use_simulator',
            default_value='True',
            description='Whether to start the simulator'
        ),
        DeclareLaunchArgument(
            name='paused',
            default_value='true'
        ),
        DeclareLaunchArgument(
            name='use_sim_time',
            default_value='true'
        ),
        DeclareLaunchArgument(
            name='gui',
            default_value='true'
        ),
        DeclareLaunchArgument(
            name='debug',
            default_value='false'
        ),
        DeclareLaunchArgument(
            name='hang_robot',
            default_value='false'
        ),
        DeclareLaunchArgument(
            name='use_lidar',
            default_value='false'
        ),
        DeclareLaunchArgument(
            name='rname',
            default_value='cyberdog'
        ),
        DeclareLaunchArgument(
            name='wname',
            default_value='simple'
        ),
        OpaqueFunction(function=launch_setup)
    ])
    return ld
