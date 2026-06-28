

# find_package(yaml_cpp_vendor REQUIRED)


option(PARAM_TEST "Build test programe for ParamHandler")


# Use system yaml-cpp (installed in /usr/local)
find_package(yaml-cpp REQUIRED)
set(YAML_CPP_INCLUDE_DIR ${YAML_CPP_INCLUDE_DIR})
set(YAML_CPP_LIBRARIES ${YAML_CPP_LIBRARIES})

include_directories(${YAML_CPP_INCLUDE_DIR})




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
