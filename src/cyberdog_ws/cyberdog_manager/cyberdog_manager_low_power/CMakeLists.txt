cmake_minimum_required(VERSION 3.8)
project(cyberdog_manager_low_power)

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
find_package(std_srvs REQUIRED)
find_package(cyberdog_utils_common REQUIRED)

include_directories(include)

install(
  DIRECTORY include/
  DESTINATION include/
)

# install(
#   DIRECTORY lib/
#   DESTINATION lib/
# )
add_executable(${PROJECT_NAME} src/power_consumption_manager.cpp src/main.cpp)
ament_target_dependencies( ${PROJECT_NAME}
  rclcpp
  protocol
  cyberdog_utils_common
  std_srvs)

install(TARGETS ${PROJECT_NAME}
  DESTINATION lib/${PROJECT_NAME})



ament_package()
