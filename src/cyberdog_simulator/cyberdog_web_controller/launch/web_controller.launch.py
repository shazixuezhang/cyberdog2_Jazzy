from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='cyberdog_web_controller',
            executable='web_controller_server.py',
            name='web_controller',
            output='screen'
        )
    ])
