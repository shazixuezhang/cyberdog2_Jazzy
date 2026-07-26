cmake_minimum_required(VERSION 3.8)
project(cyberdog_manager_core)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(cyberdog_manager_base REQUIRED)
find_package(cyberdog_protocol REQUIRED)
find_package(cyberdog_utils_system REQUIRED)
find_package(cyberdog_utils_common REQUIRED)
find_package(cyberdog_manager_black_box REQUIRED)
find_package(cyberdog_utils_machine REQUIRED)
find_package(std_srvs REQUIRED)
find_package(cyberdog_protocol REQUIRED)
find_package(cyberdog_manager_low_power REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(cyberdog_manager_user_info REQUIRED)

include_directories(include)

add_executable(${PROJECT_NAME} src/cyberdog_manager_core.cpp src/main.cpp)

# MESSAGE('--------------${CMAKE_INSTALL_PREFIX}------------------')

ament_target_dependencies( ${PROJECT_NAME}
  rclcpp
  rclcpp_action
  protocol
  cyberdog_manager_base
  cyberdog_utils_system
  cyberdog_utils_common
  cyberdog_manager_black_box
  cyberdog_utils_machine
  std_srvs
  protocol
  cyberdog_manager_low_power
  ament_index_cpp
  cyberdog_manager_user_info )

install(TARGETS ${PROJECT_NAME}
  DESTINATION lib/${PROJECT_NAME})



ament_package()
