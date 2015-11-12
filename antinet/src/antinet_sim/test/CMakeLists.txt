cmake_minimum_required(VERSION 2.8.3)

add_subdirectory(googletest/googletest)

include_directories(${gtest_SOURCE_DIR}/include)
#message("include dir: ${gtest_SOURCE_DIR}/include")
include_directories(..)

set(CMAKE_CXX_FLAGS "-std=c++11 -g3 -O0 -Wall -pedantic")

add_executable(run_tests
	Main_TestAll.cpp
	c_geometry_test.cpp)


target_link_libraries(run_tests
	gtest gtest_main)