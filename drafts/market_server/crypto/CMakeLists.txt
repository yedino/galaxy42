cmake_minimum_required(VERSION 2.8.3)
project(crypto)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(crypto ${SOURCE_FILES} c_crypto.hpp libmath.hpp sha512.cpp sha512.hpp)