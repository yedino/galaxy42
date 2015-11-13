cmake_minimum_required(VERSION 2.8.3)

add_subdirectory(googletest/googletest)

include_directories(${gtest_SOURCE_DIR}/include)
include_directories(..)

set(CMAKE_CXX_FLAGS "-std=c++11 -g3 -O0 -Wall -pedantic")

file(GLOB TEST_SOURCE_FILES "*.cpp" # All files in directory:
)

add_executable(run_tests
#	Main_TestAll.cpp
	${SIM_SOURCES}
	${TEST_SOURCE_FILES})

set_target_properties(run_tests
	PROPERTIES
	RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/test")
	
target_link_libraries(run_tests
	gtest gtest_main
	alleg
	png
	crypto
	cryptopp
	alleggl
	GLU
	GL
	boost_serialization)
