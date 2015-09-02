cmake_minimum_required(VERSION 2.8.9)
project(api_net_asio)

set(CMAKE_CXX_FLAGS "-pedantic -Wall -std=c++11")
include_directories(..)
file(GLOB SOURCE_FILES *.cpp
)

add_executable(asio-test ${SOURCE_FILES})

target_link_libraries(asio-test 
	boost_system
 pthread)
