cmake_minimum_required(VERSION 3.8)
project(cyberdog_utils_debug)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(cyberdog_utils_common REQUIRED)

install(DIRECTORY include/
  DESTINATION include/)


ament_export_include_directories(include)
# ament_export_dependencies(${dependencies})
ament_package()
