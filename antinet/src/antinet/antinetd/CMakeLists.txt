cmake_minimum_required(VERSION 2.8.3)
project (antinet)


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR})

ADD_CUSTOM_TARGET(debug
	COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=debug ${CMAKE_SOURCE_DIR}
	COMMAND make)

ADD_CUSTOM_TARGET(release
	COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=release ${CMAKE_SOURCE_DIR}
	COMMAND make)

if(CMAKE_BUILD_TYPE STREQUAL "debug")
	message("Debug build")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O0 -Wall -Wextra -pedantic -Wno-unused-parameter -Wno-unused-variable")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "release")
	message("Release build")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -Wall -Wextra -pedantic -DNDEBUG")
endif()

file(GLOB SOURCE_FILES "*.cpp" # All files in directory:
)


file(GLOB HEADERS_FILES "*.hpp" # All files in directory:
)

# remove style files
set(SIM_SOURCES ${SOURCE_FILES})
list(REMOVE_ITEM SIM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/main.cpp)
list(REMOVE_ITEM SIM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/style_tab.cpp)
list(REMOVE_ITEM SIM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/style_tab2.cpp)
list(REMOVE_ITEM SIM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/style.cpp)
#list(REMOVE_ITEM SIM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/draft_net2.cpp)
#list(REMOVE_ITEM SIM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/draft_main_rafal.cpp)

option(USE_OPENGL "Enable/Disable opengl" ON)
if(USE_OPENGL)
       add_definitions(-DUSE_OPENGL=1)
else()
       add_definitions(-DUSE_OPENGL=0)
endif()


add_library(antinet
	${SIM_SOURCES})

add_executable(antinet.elf
	main.cpp)

add_custom_target(run
	COMMAND ./antinet.elf
	DEPENDS antinet.elf
	WORKING_DIRECTORY ./)

add_custom_target(unit_test
	COMMAND ./test/run_tests
	DEPENDS test/run_tests antinet.elf
	WORKING_DIRECTORY ./)

target_link_libraries(antinet.elf
	antinet
	alleg
	png
	crypto
	cryptopp
	alleggl
	GLU
	GL
	boost_serialization)

add_subdirectory(test)

# agl was here.

