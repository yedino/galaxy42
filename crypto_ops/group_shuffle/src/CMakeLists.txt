cmake_minimum_required(VERSION 2.8.9)
project(group_shuffle)

add_subdirectory(asio)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -std=c++11 -pthread")

file(GLOB SOURCE_FILES *.cpp
)

add_executable(
        group_shuffle ${SOURCE_FILES}
)