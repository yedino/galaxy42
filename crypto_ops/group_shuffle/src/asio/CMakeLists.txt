cmake_minimum_required(VERSION 2.8.9)
project(api_net_asio)

set(CMAKE_CXX_FLAGS "-std=c++11 -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef")
include_directories(..)

add_library(net_node STATIC
	c_net_node.cpp
	c_session.cpp
	c_protocol.cpp)

add_executable(protocol-test main.cpp)
add_executable(net-node-test net_node_test.cpp)


target_link_libraries(net_node
	boost_system
	pthread)
target_link_libraries(protocol-test
	net_node)
target_link_libraries(net-node-test
	net_node)