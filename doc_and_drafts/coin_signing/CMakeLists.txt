cmake_minimum_required(VERSION 2.8)
project(coin_signing)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -pthread -g")

file(GLOB SOURCE_ED "../../crypto_ops/crypto/ed25519_src/*.c" # All files in directory:
    )

file(GLOB SOURCE_PTEST "ptest/*.cpp")

set(SOURCE_FILES main.cpp tests.cpp c_token.cpp c_wallet.cpp c_mint.cpp c_user.cpp c_netuser.cpp
        c_rpc_bitwallet.cpp c_evidences.cpp
        ../../crypto_ops/crypto/c_encryption.cpp
        ../../crypto_ops/crypto/sha_src/sha512.cpp
        ${SOURCE_PTEST}
        ${SOURCE_ED}
)
add_executable(coin_signing ${SOURCE_FILES})

find_package(Boost COMPONENTS system REQUIRED)
find_package(Boost 1.58 REQUIRED system serialization)

TARGET_LINK_LIBRARIES(coin_signing ${Boost_LIBRARIES} bitcoinapi
                        jsoncpp
                        jsonrpccpp-common
                        jsonrpccpp-client
)
