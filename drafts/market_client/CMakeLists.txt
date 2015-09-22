cmake_minimum_required(VERSION 2.8)
project(market_client)


set(CMAKE_CXX_FLAGS "-std=c++11 -g3 -Wall -pipe -Ofast") # -pedantic -Werror -Waddress")
include_directories(${Boost_INCLUDE_DIR})
#add_library(c_crypto $<TARGET_OBJECTS:c_crypto> .rsa_test)
#add_executable(c_crypto $<TARGET_OBJECTS:c_crypto> .rsa_test/)

file(GLOB SOURCE_ED "../../crypto_ops/crypto/ed25519_src/*.c" # All files in directory:
)

add_executable(market_client test.cpp c_market_client.cpp c_connect.cpp
	       ../../crypto_ops/crypto/c_encryption.cpp
	       ../../crypto_ops/crypto/sha_src/sha512.cpp
	       ../../crypto_ops/crypto/c_crypto_ed25519.cpp
	       ${SOURCE_ED}
)


find_package(Boost COMPONENTS system REQUIRED)

TARGET_LINK_LIBRARIES(market_client ${Boost_LIBRARIES} )

install(TARGETS market_client RUNTIME DESTINATION bin)
	
target_link_libraries(market_client
 	pthread
)

add_custom_target( run_market_client
	COMMAND market_client
	DEPENDS market_client	
)
