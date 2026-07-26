cmake_minimum_required(VERSION 3.8)
project(cyberdog_protocol)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "time -v")
# set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "time -v")

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(std_msgs REQUIRED)

find_package(builtin_interfaces REQUIRED)
find_package(action_msgs REQUIRED)

include(compile_lcm.cmake)
include(compile_ros.cmake)

set(lcm_protocol_list)                            # lcm 协议列表
get_lcm_protocol_file(${PROJECT_NAME} "*")        # 加载 lcm 协议
# get_lcm_protocol_file(${PROJECT_NAME} "motion")   # 仅加载 motion.lcm 协议
lcm_generate_interfaces(${PROJECT_NAME} ${lcm_protocol_list})

set(ros_protocol_list)                            # ros 协议列表
get_ros_protocol_file(${PROJECT_NAME} "msg")      # 加载 msg 协议
get_ros_protocol_file(${PROJECT_NAME} "srv")      # 加载 srv 协议
get_ros_protocol_file(${PROJECT_NAME} "action")   # 加载 action 协议
rosidl_generate_interfaces(${PROJECT_NAME} ${ros_protocol_list}
  DEPENDENCIES builtin_interfaces sensor_msgs geometry_msgs action_msgs
)


ament_export_dependencies(rosidl_default_runtime builtin_interfaces sensor_msgs geometry_msgs std_msgs)

ament_package()
