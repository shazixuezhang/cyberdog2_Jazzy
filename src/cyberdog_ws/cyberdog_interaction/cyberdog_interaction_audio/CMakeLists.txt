cmake_minimum_required(VERSION 3.8)
project(cyberdog_interaction_audio)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -g)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(rclcpp_components REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(cyberdog_utils_common REQUIRED)
find_package(cyberdog_third_party_xpack REQUIRED)
find_package(cyberdog_protocol REQUIRED)
find_package(cyberdog_utils_machine REQUIRED)
find_package(cyberdog_motion_utils REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(cyberdog_manager_user_info REQUIRED)
find_package(cyberdog_utils_debug REQUIRED)

set(dependencies
  rclcpp
  rclcpp_action
  rclcpp_components
  std_msgs
  std_srvs
  cyberdog_utils_common
  xpack
  protocol
  cyberdog_utils_machine
  cyberdog_motion_utils
  ament_index_cpp
  cyberdog_manager_user_info
  cyberdog_utils_debug
)

# SET(RUN_LCMDATA_CMAKE_FILE "${CMAKE_BINARY_DIR}/lcm_data.cmake")
# FILE(WRITE ${RUN_LCMDATA_CMAKE_FILE}
# "SET(ENV{LANG en})\n")
# FILE(APPEND ${RUN_LCMDATA_CMAKE_FILE}
# "EXECUTE_PROCESS(COMMAND lcm-gen -x ${CMAKE_SOURCE_DIR}/lcm_data.lcm WORKING_DIRECTORY \"${CMAKE_BINARY_DIR}\")\n")

execute_process(COMMAND lcm-gen -x ${CMAKE_SOURCE_DIR}/lcm_data.lcm WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

include_directories(include ${CMAKE_BINARY_DIR})
add_executable(cyberdog_interaction_audio src/main.cpp
  src/cyberdog_interaction_audio.cpp)
target_link_libraries(cyberdog_interaction_audio
${cyberdog_log_LIBRARIES}
lcm
)
target_include_directories(cyberdog_interaction_audio PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>)
target_compile_features(cyberdog_interaction_audio PUBLIC c_std_99 cxx_std_17)  # Require C99 and C++17
ament_target_dependencies(
  cyberdog_interaction_audio
  ${dependencies}
)

install(TARGETS cyberdog_interaction_audio
  DESTINATION lib/${PROJECT_NAME})


ament_export_include_directories(include)
ament_export_dependencies(${dependencies})
ament_package()
