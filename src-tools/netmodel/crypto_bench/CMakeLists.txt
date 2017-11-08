cmake_minimum_required (VERSION 3.0)
project (sodium_tests)

set (CMAKE_CXX_FLAGS "-std=c++14 -Wall -Wextra -pedantic -O3")

add_executable (sodium_tests sodium_tests.cpp affinity.cpp)
target_link_libraries (sodium_tests pthread sodium)

