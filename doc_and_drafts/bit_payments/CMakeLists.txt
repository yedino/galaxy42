cmake_minimum_required(VERSION 2.8)
project(bit_payments)
include(ExternalProject)

#FIND_PACKAGE(JSONCPP REQUIRED)
#FIND_PACKAGE(JSONRPCCPP REQUIRED)
#FIND_PACKAGE(CURL REQUIRED)
ExternalProject_Add(libbitcoin-api-cpp SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/bitcoin-api-cpp 
                    BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/bitcoin-api-cpp 
		    BUILD_COMMAND cmake . && make
		    # Disable install step
		    INSTALL_COMMAND ""
)

#add_subdirectory(bitcoin-api-cpp)

set(CMAKE_CXX_FLAGS "-std=c++11 -g3 -O0 -Wall -pedantic")
add_executable(bit_payments test.cpp c_bit_payments.cpp
)

add_dependencies(bit_payments libbitcoin-api-cpp)


target_link_libraries(bit_payments
			${CMAKE_CURRENT_BINARY_DIR}/bitcoin-api-cpp/src/api/libbitcoinapi.a 
                        jsonrpccpp-common
                        jsonrpccpp-client
)


install(TARGETS bit_payments RUNTIME DESTINATION bin)
