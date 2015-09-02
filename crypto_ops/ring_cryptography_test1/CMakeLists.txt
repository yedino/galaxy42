cmake_minimum_required(VERSION 3.2)
project(ring_cryptography)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Og -std=c++11")

set(SOURCE_FILES main.cpp
        c_user.cpp c_user.hpp
        c_ring_shuffle.cpp c_ring_shuffle.hpp
        c_logger.cpp c_logger.hpp #temporary network driver
        c_simulation.cpp c_simulation.hpp
        ../crypto/c_crypto_RSA.hpp #crypto libs
        ../crypto/libmath.hpp ../crypto/c_random_generator.hpp #other libs
)
add_executable(ring_cryptography ${SOURCE_FILES} cyclic_list.hpp)