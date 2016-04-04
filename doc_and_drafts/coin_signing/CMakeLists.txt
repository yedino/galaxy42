cmake_minimum_required(VERSION 2.8)
project(coin_signing)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Weffc++ -pthread -g -Wall -Wextra -Wpedantic")

#set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
#find_package(JSONCPP)
#if(JSONCPP_FOUND)
#    message("INFO: we found LibJsoncpp on your pc.")
#    message(JSONCPP_FOUND = ${JSONCPP_FOUND})
#    message(JSONCPP_INCLUDE_DIR = ${JSONCPP_INCLUDE_DIR})
#    message(JSONCPP_LIBRARY = ${JSONCPP_LIBRARY})
#else(JSONCPP_FOUND)
#    message("WARNING: we couldn't find LibJsoncpp on your pc.")
#endif(JSONCPP_FOUND)

file(GLOB SOURCE_ED "../../crypto_ops/crypto/ed25519_src/*.c") # All files in directory:

file(GLOB SOURCE_PTEST "ptest/*.cpp")
list(REMOVE_ITEM SOURCE_PTEST ${CMAKE_CURRENT_SOURCE_DIR}/ptest/demo.cpp)

file(GLOB SOURCE_FILES "*.cpp") # All files in directory:
list(REMOVE_ITEM SOURCE_FILES ${CMAKE_CURRENT_SOURCE_DIR}/c_rpc_bitwallet.cpp)


set(SOURCE
        serialize/jsoncpp/jsoncpp.cpp
        serialize/c_json_serializer.cpp
        coinsign_error.hpp
        ../../crypto_ops/crypto/crypto_ed25519.cpp
        ../../crypto_ops/crypto/sha_src/sha512.cpp
		network/c_TCPasync.cpp
        network/c_netuser.cpp
        network/c_connection_base.cpp
        network/c_tcp_asio_node.cpp
        ${SOURCE_FILES}
        ${SOURCE_PTEST}
        ${SOURCE_ED}
)
add_executable(coin_signing ${SOURCE})

find_package(Boost COMPONENTS system REQUIRED)
find_package(Boost 1.55 REQUIRED system serialization)


TARGET_LINK_LIBRARIES(coin_signing
                        ${Boost_LIBRARIES}
                        #bitcoinapi$
                        #${JSONCPP_LIBRARIES}
                        #jsonrpccpp-common
                        #jsonrpccpp-client
)

add_custom_target(run
	COMMAND ./coin_signing
	DEPENDS coin_signing
	WORKING_DIRECTORY ${CMAKE_PROJECT_DIR})
