cmake_minimum_required(VERSION 2.8.3)

# http://permalink.gmane.org/gmane.comp.programming.tools.cmake.user/15952
# If the user specifies -DCMAKE_BUILD_TYPE on the command line, take their definition
# and dump it in the cache along with proper documentation, otherwise set CMAKE_BUILD_TYPE
# to Debug prior to calling PROJECT()
#
IF(DEFINED CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE STRING
   "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
ELSE()
   SET(CMAKE_BUILD_TYPE Debug CACHE STRING # <--- the default build type is here!
   "Choose the type of build, options are: None(CMAKE_CXX_FLAGS or CMAKE_C_FLAGS used) Debug Release RelWithDebInfo MinSizeRel.")
ENDIF()

project (ipbench)

message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")

add_compile_options(-Wno-unused-command-line-argument)

#add local gtest
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../googletest ${CMAKE_CURRENT_BINARY_DIR}/googletest)
include_directories(../googletest/googletest/include/)

#add local jsoncpp
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../jsoncpp ${CMAKE_CURRENT_BINARY_DIR}/jsoncpp)
include_directories(../jsoncpp/include)
#link_directories(../jsoncpp/src/lib_json)

#add local local lib sodiumpp
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../sodiumpp ${CMAKE_CURRENT_BINARY_DIR}/sodiumpp)
include_directories(../sodiumpp/sodiumpp/include)

#add local lib sidh
#flags for SIDH.h
add_definitions(-D_AMD64_ -D__LINUX__) #TODO set this flags automatically
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../libsidhms/SIDH ${CMAKE_CURRENT_BINARY_DIR}/libsidhms)
include_directories(../libsidhms/SIDH)

#add local ntru sign lib
set(NTRU_SIGN_DIR "../ntru-crypto/reference-code/C/Sign/PASS/src")
include_directories("${NTRU_SIGN_DIR}")
file(GLOB ntru-sign-files ${NTRU_SIGN_DIR}/*.c)
file(GLOB BENCH ${NTRU_SIGN_DIR}/bench.c)
list(REMOVE_ITEM ntru-sign-files ${BENCH})
add_library(ntrusign ${ntru-sign-files})
target_link_libraries(ntrusign fftw3 m)


#add local build of some external libs
link_directories( build_extra/ntru/.libs )
include_directories( build_extra )


set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Wextra -pedantic")
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


# more directories of my main code
file(GLOB SOURCES_GROUP_CRYPTO crypto/*.cpp)


set(CMAKE_CXX_FAST_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O2 -Wno-unused-parameter -Wno-unused-variable")

if(CMAKE_BUILD_TYPE STREQUAL "Fast")
        message("Fast build")
        add_definitions(-DRELEASEMODE_ -DNDEBUG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FAST_FLAGS} -g0 -Ofast")

elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message("Debug build")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O1")

elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        message("Release build")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O2 -DNDEBUG -DRELEASEMODE")
else()
    message("bad CMAKE_BUILD_TYPE flag")
    message("usage is: cmake [fast/debug/release] .")
    message( FATAL_ERROR "error" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FAST_FLAGS}")
endif()

include_directories(../antinet/src/antinet_sim/)
# add_executable(ipclient.elf ipclient.cpp counter.cpp)
# add_executable(ipserver.elf ipserver.cpp)
add_executable(tunserver.elf tunserver.cpp)

target_link_libraries(tunserver.elf pthread tunserver boost_system boost_filesystem boost_program_options sodium sodiumpp ntruencrypt ntrusign jsoncpp_lib_static sidh)


add_library(tunserver counter.cpp cjdns-code/NetPlatform_linux.c c_ip46_addr.cpp
	c_peering.cpp strings_utils.cpp haship.cpp testcase.cpp protocol.cpp libs0.cpp filestorage.cpp ../antinet/src/antinet_sim/c_tnetdbg.cpp
	trivialserialize.cpp glue_lockedstring_trivialserialize.cpp crypto-sodium/ecdh_ChaCha20_Poly1305.cpp ntrupp.cpp
	generate_config.cpp text_ui.cpp c_json_load.cpp c_json_genconf.cpp
	rpc/rpc.cpp rpc/c_connection_base.cpp rpc/c_tcp_asio_node.cpp ${SOURCES_GROUP_CRYPTO})

#tests
file(GLOB TEST_SOURCES "test/*.cpp")
#debug version
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_BASE_FLAGS}")
add_executable(test-debug.elf ${TEST_SOURCES})
target_link_libraries(test-debug.elf tunserver boost_system boost_filesystem gtest sodium sodiumpp ntruencrypt jsoncpp_lib_static sidh ntrusign)
#release version
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_BASE_FLAGS} -DRELEASEMODE=1")
add_executable(test-release.elf ${TEST_SOURCES})
target_link_libraries(test-release.elf tunserver boost_system boost_filesystem gtest sodium sodiumpp ntruencrypt jsoncpp_lib_static sidh ntrusign)

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
