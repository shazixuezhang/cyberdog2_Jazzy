import os
from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch.conditions import IfCondition  # 新增

def generate_launch_description():
    pkg_nav2_bringup = get_package_share_directory('nav2_bringup')
    pkg_cyberdog_nav2 = get_package_share_directory('cyberdog_nav2_bringup')

    use_sim_time = LaunchConfiguration('use_sim_time')
    autostart = LaunchConfiguration('autostart')
    params_file = LaunchConfiguration('params_file')
    use_rviz = LaunchConfiguration('use_rviz')  # 新增：是否启动 RViz
    
    declare_use_sim_time = DeclareLaunchArgument(
        'use_sim_time', default_value='true',
        description='使用仿真时间'
    )
    
    declare_autostart = DeclareLaunchArgument(
        'autostart', default_value='true',
        description='自动启动 Nav2 生命周期'
    )
    
    declare_params_file = DeclareLaunchArgument(
        'params_file',
        default_value=os.path.join(pkg_cyberdog_nav2, 'config', 'nav2_params.yaml'),
        description='导航参数文件路径'
    )

    # 新增：RViz 开关
    declare_use_rviz = DeclareLaunchArgument(
        'use_rviz', default_value='false',  # 默认关闭
        description='是否启动 RViz2'
    )

    # Nav2 启动（SLAM 模式）
    bringup_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_nav2_bringup, 'launch', 'bringup_launch.py')
        ),
        launch_arguments={
            'slam': 'True',
            'use_sim_time': use_sim_time,
            'params_file': params_file,
            'autostart': autostart,
        }.items()
    )

    # RViz2 - 默认不启动，需要时传参
    rviz_cmd = Node(
        condition=IfCondition(use_rviz),  # 条件启动
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', os.path.join(pkg_cyberdog_nav2, 'rviz', 'nav2_view.rviz')],
        parameters=[{'use_sim_time': use_sim_time}]
    )

    return LaunchDescription([
        declare_use_sim_time,
        declare_autostart,
        declare_params_file,
        declare_use_rviz,
        bringup_cmd,
        rviz_cmd
    ])