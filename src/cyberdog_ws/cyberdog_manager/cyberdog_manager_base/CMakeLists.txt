cmake_minimum_required(VERSION 3.8)
project(cyberdog_manager_base)

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
find_package(rclcpp REQUIRED)
find_package(cyberdog_protocol REQUIRED)
find_package(cyberdog_utils_system REQUIRED)


install(DIRECTORY include/
  DESTINATION include/)


ament_export_include_directories(include)

ament_package()
