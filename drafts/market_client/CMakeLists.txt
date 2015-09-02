cmake_minimum_required(VERSION 2.8)
project(market_client)


set(CMAKE_CXX_FLAGS "-std=c++11 -g3 -Wall -pipe -Og") # -pedantic -Werror -Waddress")
include_directories(${Boost_INCLUDE_DIR})
#add_library(c_crypto $<TARGET_OBJECTS:c_crypto> .rsa_test)
#add_executable(c_crypto $<TARGET_OBJECTS:c_crypto> .rsa_test/)

file(GLOB SOURCE_ED "../ed25519/src/*.c" # All files in directory:
    )

add_executable(market_client test.cpp c_market_client.cpp c_encryption.cpp c_connect.cpp  ../../antinet/src/sha512.cpp ${SOURCE_ED})


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




