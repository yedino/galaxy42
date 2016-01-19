cmake_minimum_required(VERSION 2.8)
project(contract_signature)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

file(GLOB SOURCE_ED "../../crypto_ops/crypto/ed25519_src/*.c" # All files in directory:
)
      

set(SOURCE_FILES main.cpp 
                 c_contract.cpp 
                 c_user.cpp
                 ../../crypto_ops/crypto/c_crypto_ed25519.cpp #ed25519 signature
                 c_wallet.cpp
                 c_token.cpp
                 c_mint.cpp
                 ${SOURCE_ED}
)
add_executable(contract_signature ${SOURCE_FILES})
