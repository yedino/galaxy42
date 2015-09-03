cmake_minimum_required(VERSION 2.8.9)
project(api_net_asio)

set(CMAKE_CXX_FLAGS "-std=c++11 -pedantic -Wall -Wextra -Wcast-align -Wcast-qual -Wctor-dtor-privacy -Wdisabled-optimization -Wformat=2 -Winit-self -Wlogical-op -Wmissing-declarations -Wmissing-include-dirs -Wnoexcept -Wold-style-cast -Woverloaded-virtual -Wredundant-decls -Wshadow -Wsign-conversion -Wsign-promo -Wstrict-null-sentinel -Wstrict-overflow=5 -Wswitch-default -Wundef")
include_directories(..)
file(GLOB SOURCE_FILES *.cpp
)

add_executable(asio-test ${SOURCE_FILES})

target_link_libraries(asio-test 
	boost_system
 pthread)
