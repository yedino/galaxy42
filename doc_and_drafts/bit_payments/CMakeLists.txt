cmake_minimum_required(VERSION 2.8)
project(bit_payments)

set(CMAKE_CXX_FLAGS "-std=c++11 -g3 -O0 -Wall -pedantic -Werror")
add_executable(bit_payments test.cpp c_bit_payments.cpp)

target_link_libraries(bit_payments
	bitcoinapi)

install(TARGETS bit_payments RUNTIME DESTINATION bin)