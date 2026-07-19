


option(PARAM_TEST "Build test programe for ParamHandler")


# select version of yaml-cpp for different build methods
if(BUILD_ROS)
  # In ROS2, use system yaml-cpp via pkg-config or find_package
  find_package(yaml-cpp REQUIRED)
elseif(NOT BUILD_ROS)
  # Use system yaml-cpp (version 0.8.0 or 0.9.0 in Ubuntu 24.04)
  find_package(yaml-cpp REQUIRED)
endif(BUILD_ROS)



###
### Library
###
add_library(param_handler SHARED
  ParamHandler.hpp
  ParamHandler.cpp
)

target_include_directories(param_handler PUBLIC
  ${YAML_CPP_INCLUDE_DIR}
)

target_link_libraries(param_handler
  ${YAML_CPP_LIBRARIES}
)

if (PARAM_TEST)
  add_executable(test_param
    test/test_param_main.cpp
  )

  target_link_libraries(test_param
    param_handler
  )
endif()
