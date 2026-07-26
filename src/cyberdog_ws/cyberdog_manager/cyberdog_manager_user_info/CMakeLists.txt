cmake_minimum_required(VERSION 3.8)
project(cyberdog_manager_user_info)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Wno-class-memaccess)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(cyberdog_third_party_rapidjson REQUIRED)
find_package(cyberdog_utils_common REQUIRED)
find_package(cyberdog_manager_black_box REQUIRED)

set(dependencies
  cyberdog_utils_common
  rapidjson
  rclcpp
  cyberdog_manager_black_box
)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(include)

add_executable(test_node src/test_inteface_node.cpp)
add_executable(test_node2 test/test.cpp)

#target_compile_definitions(test_node PRIVATE BenchmarkPath="${CMAKE_INSTALL_PREFIX}/lib/cyberdog_utils_common")
ament_target_dependencies(test_node rclcpp rapidjson cyberdog_utils_common cyberdog_manager_black_box)
ament_target_dependencies(test_node2 rclcpp rapidjson cyberdog_utils_common cyberdog_manager_black_box)

target_link_libraries(test_node ${cyberdog_log_LIBRARIES})
install(
  TARGETS test_node test_node2
  DESTINATION lib/${PROJECT_NAME}
)
install(
  DIRECTORY include/
  DESTINATION include/
)
ament_export_include_directories(include)
ament_export_dependencies(${dependencies})
ament_package()

