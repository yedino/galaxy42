cmake_minimum_required(VERSION 2.8.3)

add_subdirectory(googletest/googletest)

include_directories(${gtest_SOURCE_DIR}/include)
#message("include dir: ${gtest_SOURCE_DIR}/include")
include_directories(..)

set(CMAKE_CXX_FLAGS "-std=c++11 -g3 -O0 -Wall -pedantic")

file(GLOB SOURCE_FILES "*.cpp" # All files in directory:
)

add_executable(
        run_tests
#       Main_TestAll.cpp
        ${SIM_SOURCES}
        ${SOURCE_FILES}
        )

target_link_libraries(
        run_tests
        gtest gtest_main
        alleg
        png
        crypto
        cryptopp
        alleggl
        GLU
        GL
        boost_serialization
        )
