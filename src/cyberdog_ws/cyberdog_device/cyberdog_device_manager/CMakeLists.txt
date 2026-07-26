cmake_minimum_required(VERSION 3.8)
project(cyberdog_device_manager)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rclcpp REQUIRED)
find_package(cyberdog_manager_base REQUIRED)
find_package(cyberdog_protocol REQUIRED)
find_package(pluginlib REQUIRED)
find_package(cyberdog_utils_system REQUIRED)
find_package(cyberdog_utils_common REQUIRED)
find_package(cyberdog_device_led REQUIRED)
find_package(cyberdog_device_bms REQUIRED)
find_package(cyberdog_device_uwb REQUIRED)
find_package(cyberdog_device_touch REQUIRED)
find_package(cyberdog_utils_machine REQUIRED)
find_package(ament_index_cpp REQUIRED)

include_directories(include)

add_executable(${PROJECT_NAME}
  src/cyberdog_device_manager.cpp
  src/device_handler.cpp
  src/main.cpp)

ament_target_dependencies(${PROJECT_NAME}
  rclcpp
  std_msgs
  geometry_msgs
  protocol
  cyberdog_manager_base
  cyberdog_utils_system
  cyberdog_utils_common
  cyberdog_device_led
  cyberdog_device_bms
  cyberdog_device_uwb
  cyberdog_device_touch
  pluginlib
  cyberdog_utils_machine
  ament_index_cpp
)

install(TARGETS
  ${PROJECT_NAME}
  DESTINATION lib/${PROJECT_NAME})




# ament_export_lib
ament_package()
