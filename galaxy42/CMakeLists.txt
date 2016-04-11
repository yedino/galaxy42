cmake_minimum_required(VERSION 2.8.3)
project (ipbench)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic")

ADD_CUSTOM_TARGET(fast
		COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=fast ${CMAKE_SOURCE_DIR}
		COMMAND make)
ADD_CUSTOM_TARGET(release
		COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=release ${CMAKE_SOURCE_DIR}
		COMMAND make)
ADD_CUSTOM_TARGET(debug
		COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=debug ${CMAKE_SOURCE_DIR}
		COMMAND make)

message("CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")
if(NOT CMAKE_BUILD_TYPE)
	message("DEFAULT BUILD: Fast build")
	set(CMAKE_BUILD_TYPE fast)
endif()

if(CMAKE_BUILD_TYPE STREQUAL "fast")
        message("Fast build")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O2 -Wno-unused-parameter -Wno-unused-variable")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "debug")
        message("Debug build")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O3")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "release")
        message("Release build")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O3 -DNDEBUG")
endif()


include_directories(../../../antinet/src/antinet_sim/)
add_executable(ipclient.elf ipclient.cpp counter.cpp)
add_executable(ipserver.elf ipserver.cpp)
add_executable(tunserver.elf tunserver.cpp counter.cpp ../NetPlatform_linux.c c_ip46_addr.cpp c_peering.cpp strings_utils.cpp haship.cpp testcase.cpp protocol.cpp libs1.cpp trivialserialize.cpp)

target_link_libraries(tunserver.elf boost_program_options sodium)

#loadjason.elf jsoncpp
#add_executable(loadjson.elf loadjson.cpp)
#target_link_libraries(loadjson.elf
#                      jsoncpp
#)

add_custom_target(run
		COMMAND ./tunserver.sh
		DEPENDS ./tunserver.elf
		WORKING_DIRECTORY ./)

add_custom_target(runcli
		COMMAND ./ipclient.elf ::1 12006 ipv6 1200
		DEPENDS ./ipclient.elf
		WORKING_DIRECTORY ./)

add_custom_command(
	TARGET ipclient.elf ipserver.elf tunserver.elf
	POST_BUILD
	COMMAND chmod g-wx,o-wx ipclient.elf
	COMMAND chmod g-wx,o-wx ipserver.elf
	COMMAND chmod g-wx,o-wx tunserver.elf
	COMMAND sudo setcap_net_admin --normal -u --current -f ./tunserver.elf
	WORKING_DIRECTORY ./)
