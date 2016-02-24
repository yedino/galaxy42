cmake_minimum_required(VERSION 3.0)
project(ptest)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -pthread")

set(SOURCE_FILES demo.cpp ptest.cpp ptest.hpp suite_test_macro.hpp ./prettyprint/pretty_print.hpp test_macro.hpp)
add_executable(ptest ${SOURCE_FILES})
