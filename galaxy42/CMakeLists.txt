cmake_minimum_required(VERSION 2.8.3)
project (ipbench)

#add local gtest
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../googletest ${CMAKE_CURRENT_BINARY_DIR}/googletest)
include_directories(../googletest/googletest/include/)

#add local lib sodiumpp
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../sodiumpp ${CMAKE_CURRENT_BINARY_DIR}/sodiumpp)
include_directories(../sodiumpp/sodiumpp/include)

#add local lib sidh
#flags for SIDH.h
add_definitions(-D_AMD64_ -D__LINUX__) #TODO set this flags automatically
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../libsidhms/SIDH ${CMAKE_CURRENT_BINARY_DIR}/libsidhms)
include_directories(../libsidhms/SIDH)

#add local build of some external libs
link_directories( build_extra/ntru/.libs )
include_directories( build_extra )


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic")
set(CMAKE_CXX_BASE_FLAGS "${CMAKE_CXX_FLAGS}")

ADD_CUSTOM_TARGET(fast
		COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=fast ${CMAKE_SOURCE_DIR}
		COMMAND make)
ADD_CUSTOM_TARGET(release
		COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=release ${CMAKE_SOURCE_DIR}
		COMMAND make)
ADD_CUSTOM_TARGET(debug
		COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=debug ${CMAKE_SOURCE_DIR}
		COMMAND make)

if(NOT CMAKE_BUILD_TYPE)
	message("DEFAULT BUILD: Fast build")
	set(CMAKE_BUILD_TYPE fast)
endif()

set(CMAKE_CXX_FAST_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O2 -Wno-unused-parameter -Wno-unused-variable")

if(CMAKE_BUILD_TYPE STREQUAL "fast")
        message("Fast build")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FAST_FLAGS}")

elseif(CMAKE_BUILD_TYPE STREQUAL "debug")
        message("Debug build")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O3")

elseif(CMAKE_BUILD_TYPE STREQUAL "release")
        message("Release build")
        add_definitions(-DRELEASEMODE_ -DNDEBUG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O3")
else()
    message("bad CMAKE_BUILD_TYPE flag")
    message("usage cmake [fast/debug/release] .")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FAST_FLAGS}")
endif()

include_directories(../antinet/src/antinet_sim/)
# add_executable(ipclient.elf ipclient.cpp counter.cpp)
# add_executable(ipserver.elf ipserver.cpp)
add_executable(tunserver.elf tunserver.cpp)
target_link_libraries(tunserver.elf tunserver boost_system boost_filesystem boost_program_options sodium sodiumpp ntruencrypt sidh)

add_library(tunserver counter.cpp cjdns-code/NetPlatform_linux.c c_ip46_addr.cpp
	c_peering.cpp strings_utils.cpp haship.cpp testcase.cpp protocol.cpp libs0.cpp
	trivialserialize.cpp crypto-sodium/ecdh_ChaCha20_Poly1305.cpp crypto.cpp
	c_json_load.cpp c_json_genconf.cpp jsoncpp/jsoncpp.cpp )

#tests
file(GLOB TEST_SOURCES "test/*.cpp")
#debug version
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_BASE_FLAGS}")
add_executable(test-debug.elf ${TEST_SOURCES})
target_link_libraries(test-debug.elf tunserver gtest sodium sodiumpp ntruencrypt sidh)
#release version
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_BASE_FLAGS} -DRELEASEMODE=1")
add_executable(test-release.elf ${TEST_SOURCES})
target_link_libraries(test-release.elf tunserver gtest sodium sodiumpp ntruencrypt sidh)

#loadjason.elf jsoncpp
#add_executable(jsonload.elf
#			   c_json_load.cpp
#			   c_json_genconf.cpp
#			   jsoncpp/jsoncpp.cpp
#)
#target_link_libraries(jsonload.elf
#				tunserver
#				boost_system
#				boost_filesystem
#                sodium
#)

add_custom_target(run
		COMMAND ./tunserver.sh
		DEPENDS ./tunserver.elf
		DEPENDS test-debug.elf
		WORKING_DIRECTORY ./)

add_custom_target(runcli
		COMMAND ./ipclient.elf ::1 12006 ipv6 1200
		DEPENDS ./ipclient.elf
		WORKING_DIRECTORY ./)

add_custom_command(
	TARGET ipclient.elf ipserver.elf tunserver.elf
	POST_BUILD
#	COMMAND chmod g-wx,o-wx ipclient.elf
#	COMMAND chmod g-wx,o-wx ipserver.elf
	COMMAND chmod g-wx,o-wx tunserver.elf
	COMMAND cp tunserver.elf nocap-tunserver.elf
	COMMAND sudo setcap_net_admin --normal -u --current -f ./tunserver.elf
	WORKING_DIRECTORY ./)
