cmake_minimum_required(VERSION 3.8)
project(cyberdog_interaction_ai_sports)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 14)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(cyberdog_protocol REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(cyberdog_params REQUIRED)
find_package(std_msgs REQUIRED)
find_package(cyberdog_utils_system REQUIRED)
find_package(cyberdog_utils_common REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(rclcpp_lifecycle REQUIRED)
find_package(lifecycle_msgs REQUIRED)

# include_directories(include)
add_executable(${PROJECT_NAME}
  src/main.cpp
  src/cyberdog_interaction_ai_sports.cpp
  src/calculate_angle.cpp
  src/sport_counts.cpp)

target_link_libraries(${PROJECT_NAME}
  ${cyberdog_log_LIBRARIES})

target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(${PROJECT_NAME} PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
ament_target_dependencies(
  ${PROJECT_NAME}
  rclcpp
  protocol
  ament_index_cpp
  params
  sensor_msgs
  cyberdog_utils_system
  cyberdog_utils_common
  lifecycle_msgs
  rclcpp_lifecycle
  std_msgs)

install(TARGETS ${PROJECT_NAME}
  DESTINATION lib/${PROJECT_NAME})


ament_package()
