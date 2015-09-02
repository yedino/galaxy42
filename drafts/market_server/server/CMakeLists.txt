cmake_minimum_required(VERSION 2.8.3)
project(server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -std=c++14 -lboost_system -pthread")

set(SOURCE_FILES main.cpp)
add_executable(server ${SOURCE_FILES} ../market/c_market.cpp ../market/c_market.hpp ../crypto/c_crypto.hpp ../crypto/libmath.hpp ../crypto/sha512.cpp ../crypto/sha512.hpp c_market_server.cpp c_market_server.hpp ../hash_table/hash_table.hpp ../hash_table/hash_table.cpp)

find_package(Boost COMPONENTS system REQUIRED)

TARGET_LINK_LIBRARIES(server ${Boost_LIBRARIES})