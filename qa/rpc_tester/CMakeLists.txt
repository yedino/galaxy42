cmake_minimum_required(VERSION 3.1)
project (rpc_tester)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")
find_package(Boost REQUIRED COMPONENTS system program_options)
find_package(Sodium)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")

add_executable(rpc_tester main.cpp)
target_link_libraries(rpc_tester boost_system boost_program_options sodium)
