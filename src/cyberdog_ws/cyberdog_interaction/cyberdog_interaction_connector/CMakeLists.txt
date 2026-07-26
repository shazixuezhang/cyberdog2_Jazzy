cmake_minimum_required(VERSION 3.8)
project(cyberdog_interaction_connector)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Wno-class-memaccess)
endif()

if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_ros REQUIRED)

find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(std_srvs REQUIRED)

find_package(cyberdog_third_party_zxing REQUIRED)
find_package(cyberdog_protocol REQUIRED)
find_package(cyberdog_params REQUIRED)
find_package(bes_transmit REQUIRED)
find_package(cyberdog_utils_common REQUIRED)
find_package(cyberdog_utils_system REQUIRED)
find_package(cyberdog_third_party_rapidjson REQUIRED)

find_package(python_cmake_module REQUIRED)
find_package(PythonExtra REQUIRED)
find_package(Python 3.6 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

include_directories(include)
include(compile_and_install.cmake)
compile_and_install_node()

install(DIRECTORY script config DESTINATION share/${PROJECT_NAME})


ament_export_include_directories(include)
ament_package()
