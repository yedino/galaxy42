cmake_minimum_required(VERSION 2.8.3)
project (ipbench)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wextra -pedantic -O3 -g3")

include_directories(../../../antinet/src/antinet_sim/)
add_executable(ipclient.elf ipclient.cpp counter.cpp)
add_executable(ipserver.elf ipserver.cpp)
add_executable(tunserver.elf tunserver.cpp)

add_custom_target(run
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
