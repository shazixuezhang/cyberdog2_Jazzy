cmake_minimum_required(VERSION 3.8)
project(cyberdog_bridge_can)

# Default to C99
if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(cyberdog_third_party_toml REQUIRED)
find_package(cyberdog_utils_common REQUIRED)

set(dependencies
  toml
  cyberdog_utils_common
)

install(
  DIRECTORY include/
  DESTINATION include/
)


ament_export_include_directories(include)
ament_export_dependencies(${dependencies})
ament_package()
