cmake_minimum_required(VERSION 2.8)

project(mat_lib CXX)

#message(STATUS "libraries:")
#
#message(STATUS "  'mat_lib' will be generated")
## mat_lib library 
#add_library(
#  mat_lib
#  matrix.cpp 
#)

message(STATUS "targets:")
# testing_mat_lib 
message(STATUS "  'testing_mat_lib' will be generated")
add_executable(
  testing_mat_lib 
  testing_mat_lib.cpp
)
#target_link_libraries(testing_mat_lib mat_lib)

message(STATUS "  'testing_generic_value' will be generated")
add_executable(
  testing_generic_value 
  testing_generic_value.cpp
)

message(STATUS "  'simple_calculator_v13' will be generated")
add_executable(
  simple_calculator_v13 
  simple_calculator_v13.cpp
)

