cmake_minimum_required(VERSION 2.8)
project(contract_signature)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp
        c_contract.cpp c_contract.hpp
        c_user.cpp c_user.hpp
        c_crypto_ed25519.cpp #ed25519 signature
        c_random_generator.hpp
        ed25519_src/add_scalar.c ed25519_src/fe.c ed25519_src/ge.c ed25519_src/key_exchange.c #ed25519 libs #1
        ed25519_src/sc ed25519_src/seed ed25519_src/sign.c ed25519_src/verify.c ed25519_src/keypair.c ed25519_src/sha512 #ed25519 libs #2
        c_wallet.cpp c_wallet.hpp
        c_token.cpp c_token.hpp
        c_mint.hpp c_mint.cpp
)
add_executable(contract_signature ${SOURCE_FILES})
