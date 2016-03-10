cmake_minimum_required(VERSION 2.8.3)
project (ipbench)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic -O3 -g3")
add_executable(ipclient.elf ipclient.cpp)
add_executable(ipserver.elf ipserver.cpp)
add_executable(tunserver.elf tunserver.cpp)
