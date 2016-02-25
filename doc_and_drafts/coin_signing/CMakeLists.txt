cmake_minimum_required(VERSION 2.8)
project(coin_signing)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread -g -Wall -Wextra -Wpedantic")

file(GLOB SOURCE_ED "../../crypto_ops/crypto/ed25519_src/*.c") # All files in directory:

file(GLOB SOURCE_PTEST "ptest/*.cpp")
list(REMOVE_ITEM SOURCE_PTEST ${CMAKE_CURRENT_SOURCE_DIR}/ptest/demo.cpp)

file(GLOB SOURCE_FILES "*.cpp") # All files in directory:
list(REMOVE_ITEM SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/c_rpc_bitwallet.cpp)


set(SOURCE
        coinsign_error.hpp
        libs01.hpp
        ../../crypto_ops/crypto/crypto_ed25519.cpp
        ../../crypto_ops/crypto/sha_src/sha512.cpp
        ${SOURCE_FILES}
        ${SOURCE_PTEST}
        ${SOURCE_ED}
)
add_executable(coin_signing ${SOURCE})

find_package(Boost COMPONENTS system REQUIRED)
find_package(Boost 1.55 REQUIRED system serialization)

TARGET_LINK_LIBRARIES(coin_signing ${Boost_LIBRARIES}
                        #bitcoinapi
                        #jsoncpp
                        #jsonrpccpp-common
                        #jsonrpccpp-client
)
