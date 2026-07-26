cmake_minimum_required(VERSION 3.8)
project(cyberdog_motion_action)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -g)
endif()

if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclpy REQUIRED)
find_package(cyberdog_protocol REQUIRED)
find_package(cyberdog_utils_system REQUIRED)
find_package(cyberdog_utils_debug REQUIRED)
find_package(cyberdog_utils_common)
find_package(lcm REQUIRED)
find_package(ament_index_cpp REQUIRED)


include_directories(include)

add_library(${PROJECT_NAME} SHARED src/cyberdog_motion_action.cpp)
target_link_libraries(${PROJECT_NAME} lcm)
ament_target_dependencies(${PROJECT_NAME}
  rclcpp
  protocol
  cyberdog_utils_system
  cyberdog_utils_common
  cyberdog_utils_debug
  ament_index_cpp
)

# add_executable(manager_test test/manager_test.cpp)
# target_link_libraries(manager_test ${PROJECT_NAME} lcm)
# ament_target_dependencies(manager_test lcm cyberdog_utils_common cyberdog_utils_debug)

add_executable(controller_test test/controller_test.cpp)
target_link_libraries(controller_test ${PROJECT_NAME} lcm)
ament_target_dependencies(controller_test lcm cyberdog_utils_common cyberdog_utils_debug)

add_executable(publisher_test test/publisher_test.cpp)
target_link_libraries(publisher_test ${PROJECT_NAME} lcm)
ament_target_dependencies(publisher_test lcm cyberdog_utils_common cyberdog_utils_debug)

add_executable(client_test test/client_test.cpp)
target_link_libraries(client_test ${PROJECT_NAME} lcm)
ament_target_dependencies(client_test lcm cyberdog_utils_common cyberdog_utils_debug)

install(
  DIRECTORY include/
  DESTINATION include
)
install(
  DIRECTORY launch
  DESTINATION share/${PROJECT_NAME}
)
install(
  DIRECTORY preset
  DESTINATION share/${PROJECT_NAME}
)
install(TARGETS
  ${PROJECT_NAME}
  # manager_test
  controller_test
  publisher_test
  client_test
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

ament_python_install_package(${PROJECT_NAME})

install(PROGRAMS
  scripts/motion_teleop.py
  scripts/pose_teleop.py
  scripts/statistics.py
  scripts/motion_units.py
  scripts/motion_demo.py
  DESTINATION lib/${PROJECT_NAME}
)


ament_export_include_directories(
  include
)
ament_export_libraries(
  ${PROJECT_NAME}
)
ament_export_dependencies(lcm cyberdog_utils_common cyberdog_utils_debug)
ament_package()
