cmake_minimum_required(VERSION 3.8)
project(cyberdog_manager_black_box)

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
find_package(Boost COMPONENTS system filesystem REQUIRED)
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(cyberdog_protocol REQUIRED)
find_package(sqlite3_vendor REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(cyberdog_utils_common REQUIRED)
find_package(cyberdog_params REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

include_directories(include)

add_library(cyberdog_manager_black_box SHARED src/cyberdog_manager_black_box.cpp)
target_link_libraries(cyberdog_manager_black_box ${SQLite3_LIBRARIES} ${Boost_FILESYSTEM_LIBRARY})
ament_target_dependencies(cyberdog_manager_black_box rclcpp protocol cyberdog_utils_common params)
ament_target_dependencies(cyberdog_manager_black_box rclcpp cyberdog_utils_common params)
# target_include_directories(cyberdog_manager_black_box
#                            PUBLIC
#                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#                            $<INSTALL_INTERFACE:include>)

add_executable(test_cyberdog_manager_black_box src/test_cyberdog_manager_black_box.cpp)
target_link_libraries(test_cyberdog_manager_black_box cyberdog_manager_black_box)
ament_target_dependencies(test_cyberdog_manager_black_box rclcpp)
# target_include_directories(test_cyberdog_manager_black_box
#                            PUBLIC
#                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
#                            $<INSTALL_INTERFACE:include>)


install(DIRECTORY
        include/
        DESTINATION include)

install(TARGETS cyberdog_manager_black_box test_cyberdog_manager_black_box
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)
ament_export_include_directories(include)
ament_export_dependencies(sqlite3_vendor SQLite3 cyberdog_utils_common)
ament_export_libraries(cyberdog_manager_black_box)
ament_package()
