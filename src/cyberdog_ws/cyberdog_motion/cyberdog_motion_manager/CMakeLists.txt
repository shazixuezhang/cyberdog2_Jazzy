cmake_minimum_required(VERSION 3.8)
project(cyberdog_motion_manager)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -g)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(cyberdog_manager_base REQUIRED)
find_package(cyberdog_utils_common REQUIRED)
find_package(cyberdog_utils_system REQUIRED)
find_package(cyberdog_protocol REQUIRED)
find_package(cyberdog_motion_action REQUIRED)
find_package(cyberdog_utils_debug REQUIRED)
find_package(cyberdog_utils_machine REQUIRED)
find_package(pluginlib REQUIRED)
find_package(ament_index_cpp REQUIRED)


include_directories(include)

add_executable(${PROJECT_NAME}
  src/motion_handler.cpp
  src/motion_decision.cpp
  src/cyberdog_motion_manager.cpp
  src/main.cpp)

ament_target_dependencies(${PROJECT_NAME}
  rclcpp
  protocol
  cyberdog_manager_base
  cyberdog_utils_system
  cyberdog_utils_common
  cyberdog_motion_action
  protocol
  pluginlib
  ament_index_cpp
)

install(TARGETS
  ${PROJECT_NAME}
  DESTINATION lib/${PROJECT_NAME})

install(DIRECTORY
  config
  DESTINATION share/${PROJECT_NAME})


ament_package()
