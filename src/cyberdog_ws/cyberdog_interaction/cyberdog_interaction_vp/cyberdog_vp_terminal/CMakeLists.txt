cmake_minimum_required(VERSION 3.8)
project(cyberdog_interaction_vp_terminal)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic -Wno-class-memaccess)
endif()

if(NOT CMAKE_C_STANDARD)
  set(CMAKE_C_STANDARD 99)
endif()

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

find_package(python_cmake_module REQUIRED)

# find_package (Python3 COMPONENTS Interpreter Development)
# find_package(PythonInterp)
# if (PYTHONINTERP_FOUND)
#   if (PYTHON_VERSION_MAJOR EQUAL 3)
#     find_package(Boost COMPONENTS python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR})
#     find_package(PythonInterp 3)
#     find_package(PythonLibs 3 REQUIRED)
#     set(boost_python "boost_python${PYTHON_VERSION_MAJOR}${PYTHON_VERSION_MINOR}")
#   else()
#     find_package(Boost COMPONENTS python)
#     find_package(PythonInterp)
#     find_package(PythonLibs REQUIRED)
#     set(boost_python "boost_python")
#   endif()
# else()
#   message("Python not found")
# endif()
# message(STATUS "Boost_python = ${boost_python}")

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_ros REQUIRED)
find_package(ament_cmake_python REQUIRED)

find_package(rclcpp REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(std_srvs REQUIRED)

find_package(cyberdog_protocol REQUIRED)
find_package(cyberdog_params REQUIRED)
find_package(cyberdog_utils_common REQUIRED)
find_package(cyberdog_third_party_rapidjson REQUIRED)

find_package(cyberdog_interaction_vp_abilityset REQUIRED)

find_package(python_cmake_module REQUIRED)
find_package(PythonExtra REQUIRED)
find_package(Python 3.6 COMPONENTS Interpreter Development REQUIRED)
find_package(pybind11_vendor REQUIRED)
find_package(pybind11 CONFIG REQUIRED)

include(compile_and_install.cmake)
compile_and_install_node()
compile_and_install_api()


ament_export_include_directories(include)
ament_package()
