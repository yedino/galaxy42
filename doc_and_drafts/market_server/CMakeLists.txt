cmake_minimum_required(VERSION 3.2)
project(server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -Og -std=c++14 -lboost_system -pthread")

set(SOURCE_FILES main.cpp)
file(GLOB SOURCE_ED "../../crypto_ops/crypto/ed25519_src/*.c" # All files in directory:
)

add_executable(server   ${SOURCE_FILES}
                        ${SOURCE_ED}
        c_market.cpp
        c_market_server.cpp
        c_server_msg_parser.cpp
        c_users_database.cpp
        ../../crypto_ops/crypto/c_crypto_ed25519.cpp
        ../../crypto_ops/crypto/sha_src/sha256.cpp
        ../../crypto_ops/crypto/sha_src/sha512.cpp
)

find_package(Boost COMPONENTS system REQUIRED)

TARGET_LINK_LIBRARIES(server ${Boost_LIBRARIES})
