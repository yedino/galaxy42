cmake_minimum_required(VERSION 2.8.9)
project(crypto)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pipe -Ofast -std=c++11 -pthread")

set(SOURCE_FILES 
		main.cpp
		sha_src/sha512.cpp sha_src/sha256.cpp sha_src/sha256.hpp #sha libs
		c_crypto_ed25519.cpp #ed25519 signature
		c_crypto_geport.cpp #geport signature
		ed25519_src/add_scalar.c ed25519_src/fe.c ed25519_src/ge.c ed25519_src/key_exchange.c #ed25519 libs #1
		ed25519_src/sc ed25519_src/seed ed25519_src/sign.c ed25519_src/verify.c ed25519_src/keypair.c ed25519_src/sha512 #ed25519 libs #2)
)

add_executable(
        crypto ${SOURCE_FILES}
)

#TARGET_LINK_LIBRARIES(crypto cryptopp)

add_custom_target(run
	COMMAND ./crypto
	DEPENDS crypto
	WORKING_DIRECTORY .)