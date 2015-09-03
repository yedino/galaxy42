cmake_minimum_required(VERSION 2.8)
project(coin_signing)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp
        c_token.cpp c_token.hpp
        c_wallet.cpp c_wallet.hpp
        c_user.cpp c_user.hpp
        c_mint.cpp c_mint.hpp
        ../../crypto_ops/crypto/c_random_generator.hpp
)
add_executable(coin_signing ${SOURCE_FILES})