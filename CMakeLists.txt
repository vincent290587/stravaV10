cmake_minimum_required (VERSION 3.9.3)

project (stravaV10)

# The version number.
set (Tutorial_VERSION_MAJOR 1)
set (Tutorial_VERSION_MINOR 0)

# compilation flags
option (TDD "Use TDD functions" ON) 

# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/TEST/main_tdd.h.in"
  "${PROJECT_BINARY_DIR}/main_tdd.h"
  )
 
# add the binary tree to the search path for include files
# so that we will find TutorialConfig.h
include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_BINARY_DIR}/..")

include_directories("${PROJECT_BINARY_DIR}/../sources/routes")
#add_subdirectory (../sources/routes) 
 
# add the executable
add_executable(StravaV10 main_tdd.cpp)
#target_link_libraries (StravaV10 routes)