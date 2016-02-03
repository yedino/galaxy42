cmake_minimum_required(VERSION 3.0.2)
project(bit_payments)

#FIND_PACKAGE(JSONCPP REQUIRED)
#FIND_PACKAGE(JSONRPCCPP REQUIRED)
#FIND_PACKAGE(CURL REQUIRED)
ExternalProject_Add(libbitcoin-api-cpp)

#add_subdirectory(bitcoin-api-cpp)

set(CMAKE_CXX_FLAGS "-std=c++11 -g3 -O0 -Wall -pedantic")
add_executable(bit_payments test.cpp c_bit_payments.cpp
                bitcoin-api-cpp/src/api/bitcoinapi.cpp
)



target_link_libraries(bit_payments bitcoinapi)
    #/usr/local/lib/libbitcoinapi.so)

install(TARGETS bit_payments RUNTIME DESTINATION bin)
