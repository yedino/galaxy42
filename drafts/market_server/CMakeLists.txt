cmake_minimum_required(VERSION 3.2)
project(server)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -Og -std=c++14 -lboost_system -pthread")

set(SOURCE_FILES main.cpp)

add_executable(server ${SOURCE_FILES}
        ../market/c_market.cpp ../market/c_market.hpp
        libmath.hpp
        c_random_generator.hpp
        sha_src/sha256.cpp sha_src/sha256.hpp
        sha_src/sha512.cpp sha_src/sha512.hpp
        c_market_server.cpp c_market_server.hpp
        c_market_user.hpp
        c_server_msg_parser.cpp c_server_msg_parser.hpp
        c_crypto_ed25519.cpp c_crypto_ed25519.hpp
        c_users_database.cpp c_users_database.hpp
        ed25519_src/add_scalar.c ed25519_src/fe.c ed25519_src/ge.c ed25519_src/key_exchange.c #ed25519 libs #1
        ed25519_src/sc ed25519_src/seed ed25519_src/sign.c ed25519_src/verify.c ed25519_src/keypair.c ed25519_src/sha512 #ed25519 libs #2
)

find_package(Boost COMPONENTS system REQUIRED)

TARGET_LINK_LIBRARIES(server ${Boost_LIBRARIES})