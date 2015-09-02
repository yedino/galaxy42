cmake_minimum_required(VERSION 2.8.3)
project(market)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(market ${SOURCE_FILES} c_market.cpp c_market.hpp)