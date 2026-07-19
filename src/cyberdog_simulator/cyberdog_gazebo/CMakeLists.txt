cmake_minimum_required(VERSION 3.8)
project(cyberdog_gazebo)

# set(CMAKE_IGNORE_PATH "/usr/local")

# C标准
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()
# gz-harmonic 建议 C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(ONBOARD_BUILD)
  SET (THIS_COM "../" )
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
    ${CMAKE_BINARY_DIR}/Configuration.h)
else()
  SET (THIS_COM "${PROJECT_SOURCE_DIR}/" )
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
    ${CMAKE_BINARY_DIR}/Configuration.h)
endif()

# ROS基础依赖
find_package(ament_cmake REQUIRED)
find_package(rosidl_default_generators REQUIRED)
find_package(rosidl_typesupport_cpp REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(Eigen3 REQUIRED)
find_package(lcm REQUIRED)
find_package(cyberdog_msg REQUIRED)

# 1. 先加载所有gz vendor包
find_package(gz_cmake_vendor REQUIRED)
find_package(gz_math_vendor REQUIRED)
find_package(gz_utils_vendor REQUIRED)
find_package(gz_plugin_vendor REQUIRED)
find_package(gz_sim_vendor REQUIRED)

# 2. 加载原生gz库（vendor自动提供路径）
find_package(gz-sim REQUIRED)
find_package(gz-plugin REQUIRED)

find_package(ros_gz REQUIRED COMPONENTS ros_gz_sim ros_gz_bridge)

# 依赖列表
set(dependencies
  rclcpp
  sensor_msgs
  tf2_ros
  tf2_geometry_msgs
  Eigen3
  cyberdog_msg
  ros_gz
  gz_cmake_vendor
  gz_math_vendor
  gz_utils_vendor
  gz_plugin_vendor
  gz_sim_vendor
  gz_common_vendor
)

file(GLOB_RECURSE sources "src/control_parameters/*.cpp"
                          "src/sim_utilities/*.cpp"
                          "src/utilities/*.cpp")

# 直接获取原生库头文件目录 (hard-coded for ROS2 jazzy)
set(GZ_SIM_INC "/opt/ros/jazzy/opt/gz_sim_vendor/include/gz/sim8")
set(GZ_PLUGIN_INC "/opt/ros/jazzy/opt/gz_plugin_vendor/include/gz/plugin2")

find_package(gz_common_vendor REQUIRED)
find_package(gz-common REQUIRED)
# Hard-code gz-common include path for ROS2 jazzy
set(GZ_COMMON_INC "/opt/ros/jazzy/opt/gz_common_vendor/include")

include_directories(
  include
  include/ctrl_ros
  third-party/ParamHandler
  ${CMAKE_BINARY_DIR}
  ${GZ_SIM_INC}
  ${GZ_PLUGIN_INC}
  ${GZ_COMMON_INC}/gz/common5
)

add_subdirectory("third-party/ParamHandler")

# 链接使用原生gz目标，废弃 ${gz_sim_vendor_LIBRARIES}
add_library(foot_contact_plugin SHARED src/foot_contact_plugin.cpp)
ament_target_dependencies(foot_contact_plugin ${dependencies})
target_link_libraries(foot_contact_plugin
  gz-sim::gz-sim
  gz-plugin::gz-plugin
  lcm
)

add_library(legged_plugin SHARED
  ${sources}
  src/legged_plugin.cpp
  src/legged_simparam.cpp
  src/lcmhandler.cpp
)
ament_target_dependencies(legged_plugin ${dependencies})
target_link_libraries(legged_plugin
  gz-sim::gz-sim
  gz-plugin::gz-plugin
  param_handler
  pthread
  lcm
)

install(TARGETS legged_plugin param_handler foot_contact_plugin
    EXPORT export_${PROJECT_NAME}
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(
  DIRECTORY launch world params
  DESTINATION share/${PROJECT_NAME}
)

ament_package()
