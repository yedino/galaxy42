cmake_minimum_required(VERSION 3.0.2)
project (galaxy42)

# IMPORTANT:
# search in ALL other files (source code, the bash scripts ./do and others)
# following topics to match this topics here:
#
# match_extlevel_* - this are places that decide on which EXTLEVEL given feature is enabled:
# {{match_extlevel_sidh}} - SIDH crypto lib: now at level 30 (EXTLEVEL_IS_EXPERIMENT_DANGEROUS)
# {{match_extlevel_ntru}} - NTRU crypto lib (encr, and sign): now at level 20 (EXTLEVEL_IS_EXPERIMENT)
#

ADD_CUSTOM_TARGET(Release
		COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
		COMMAND make)
ADD_CUSTOM_TARGET(Debug
		COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
		COMMAND make)

########################### CMAKE MODULES ###########################
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cotire/CMake")
include(cotire)

if(MINGW)
	add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)
#	include_directories(depends/gettext/gettext-0.19.8/install_dir/include/)
#	link_directories(depends/gettext/gettext-0.19.8/install_dir/lib)
#	link_directories(depends/gettext/libiconv-1.14/install_dir/lib)
	set(MINGW_LINK_FLAGS "-Wl,--allow-multiple-definition")
	set(MINGW_LIBRARIES ws2_32 iphlpapi mswsock)
endif()


if (MSVC OR MINGW)
	set(SODIUM_LIB "${SODIUM_LIBRARY}")
	set(MSVC_LIRARIES "iphlpapi")
else()
	set(PTHREAD_LIB "pthread")
	set(SODIUM_LIB "sodium")
endif()

if(WIN32 OR CYGWIN)
	add_definitions(-DUNICODE)
	add_definitions(-D_UNICODE)
	add_definitions(-DWINVER=0x0601)
	add_definitions(-D_WIN32_WINNT=0x0601)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

########################### COVERAGE ###########################
IF(DEFINED COVERAGE)
   SET(COVERAGE ${COVERAGE} CACHE BOOL "")
ELSE()
   SET(COVERAGE OFF CACHE BOOL "")
ENDIF()

if(COVERAGE)
	message(STATUS "Enabling COVERAGE")
	set(COVERAGE_CXX_FLAG "--coverage")
else()
	message(STATUS "Disabling COVERAGE")
	set(COVERAGE_CXX_FLAG "")
endif()

########################### STATIC BUILD ###########################
option(BUILD_STATIC_TUNSERVER "Build the static tunserver.elf" OFF)
if(BUILD_STATIC_TUNSERVER)
	if(MSVC)
		message(FATAL_ERROR "Static build is not supportted")
	endif()
	if(NOT BOOST_INCLUDEDIR AND NOT BOOST_ROOT AND NOT BOOSTROOT)
		message(FATAL_ERROR "BOOST_INCLUDEDIR or BOOST_ROOT is required when BUILD_STATIC_TUNSERVER is ON")
	endif()

	if(NOT BOOST_LIBRARYDIR AND NOT BOOST_ROOT AND NOT BOOSTROOT)
		message(FATAL_ERROR "BOOST_LIBRARYDIR is required when BUILD_STATIC_TUNSERVER is ON")
	endif()
endif()

if(BUILD_STATIC_TUNSERVER)
	if(MSVC)
		set(STATIC_FLAGS "")
	else()
		set(STATIC_FLAGS "-fPIC -fPIE -Wl,--whole-archive -static -lpthread -Wl,--no-whole-archive")
	endif()
endif()
########################### EXTLEVEL OPTIONS ###########################
IF(DEFINED EXTLEVEL)
   SET(EXTLEVEL ${EXTLEVEL} CACHE STRING "")
ELSE()
   SET(EXTLEVEL 0 CACHE STRING "")
ENDIF()

option(EXTLEVEL_IS_NORMAL "Enable the normal stable code" ON)
if (EXTLEVEL GREATER -1) # from0
	set(EXTLEVEL_IS_NORMAL ON)
endif()

option(EXTLEVEL_IS_PREVIEW "Enable the less tested code, that is not yet recommended for any real use" OFF)
if (EXTLEVEL GREATER 9) # from 10
	set(EXTLEVEL_IS_PREVIEW ON)
endif()

option(EXTLEVEL_IS_EXPERIMENT "Enable the experimental code, that has bugs" OFF)
if (EXTLEVEL GREATER 19) # from 20
	set(EXTLEVEL_IS_EXPERIMENT ON)
endif()

option(EXTLEVEL_IS_EXPERIMENT_DANGEROUS "Enable the most experimental code, that has bugs and vulnerabilities" OFF)
if (EXTLEVEL GREATER 29) # from 30
	set(EXTLEVEL_IS_EXPERIMENT_DANGEROUS ON)
endif()

if(EXTLEVEL_IS_NORMAL)
	message(STATUS "EXTLEVEL enabling EXTLEVEL_IS_NORMAL")
	add_definitions(-DEXTLEVEL_IS_NORMAL_CMAKE=1)
else()
	add_definitions(-DEXTLEVEL_IS_NORMAL_CMAKE=0)
endif()
if(EXTLEVEL_IS_PREVIEW)
	message(STATUS "EXTLEVEL enabling EXTLEVEL_IS_PREVIEW")
	add_definitions(-DEXTLEVEL_IS_PREVIEW_CMAKE=1)
else()
	add_definitions(-DEXTLEVEL_IS_PREVIEW_CMAKE=0)
endif()
if(EXTLEVEL_IS_EXPERIMENT)
	message(STATUS "EXTLEVEL enabling EXTLEVEL_IS_EXPERIMENT !!")
	add_definitions(-DEXTLEVEL_IS_EXPERIMENT_CMAKE=1)
else()
	add_definitions(-DEXTLEVEL_IS_EXPERIMENT_CMAKE=0)
endif()
if(EXTLEVEL_IS_EXPERIMENT_DANGEROUS)
	message(STATUS "EXTLEVEL enabling EXTLEVEL_IS_EXPERIMENT !!! this code will be insecure!!!")
	add_definitions(-DEXTLEVEL_IS_EXPERIMENT_DANGEROUS_CMAKE=1)
else()
	add_definitions(-DEXTLEVEL_IS_EXPERIMENT_DANGEROUS_CMAKE=0)
endif()

set(TXT_SIDH "Enable crypto: SIDH")
if(EXTLEVEL_IS_EXPERIMENT_DANGEROUS) ## {{match_extlevel_sidh}}
	option(ENABLE_CRYPTO_SIDH TXT_SIDH ON)
else()
	option(ENABLE_CRYPTO_SIDH TXT_SIDH OFF)
endif()

set(TXT_NTRU "Enable crypto: NTru")
if(EXTLEVEL_IS_EXPERIMENT) # {{match_extlevel_ntru}}
	option(ENABLE_CRYPTO_NTRU TXT_NTRU ON)
else()
	option(ENABLE_CRYPTO_NTRU TXT_NTRU OFF)
endif()

########################### DEPENDS ###########################

########################### BOOST ###########################
set (Boost_USE_MULTITHREADED ON)
if (BUILD_STATIC_TUNSERVER)
	set (Boost_USE_STATIC_RUNTIME ON)
	set (Boost_USE_STATIC_LIBS ON)
	set (Boost_USE_DEBUG_RUNTIME OFF)
	set (sodium_USE_STATIC_LIBS ON)
endif()

if (CYGWIN)
	set(BOOST_LOCALE_LINK_LIB boost_locale-mt boost_thread-mt boost_atomic-mt)
	set(BOOST_THREAD_LINK_LIB boost_thread-mt boost_atomic-mt)
else()
	set(BOOST_LOCALE_LIB_SEARCH)
	set(BOOST_THREAD_LIB_SEARCH thread atomic)
endif()

if (MSVC)
	set(BOOST_LIBS_LIST system filesystem thread ${BOOST_LOCALE_LIB_SEARCH})
else()
	set(BOOST_LIBS_LIST ${BOOST_LOCALE_LIB_SEARCH} system filesystem program_options)
endif()
find_package(Boost 1.49.0 REQUIRED COMPONENTS ${BOOST_LIBS_LIST})

if (NOT Boost_FOUND)
	message("\nBOOST NOT FOUND!")
	if (MSVC)
		message(STATUS "\nNote (MSVC): \nPlease download and install package (or later):")
		message(STATUS "    https://sourceforge.net/projects/boost/files/boost-binaries/ boost_1_61_0-msvc-14.0-64.exe \n")
		message(STATUS "Add following argument for cmake (default install location):")
		message(STATUS "    \"-DBOOST_ROOT=C:/local/boost_1_61_0 -DBOOST_LIBRARYDIR=C:/local/boost_1_61_0/lib64-msvc-14.0/\"\n\n")
	endif()
	message(FATAL_ERROR "BOOST REQUIRED AND NOT FOUND, NOW STOPPING")
endif()

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

IF(DEFINED USE_BOOST_MULTIPRECISION_DEFAULT)
	SET(USE_BOOST_MULTIPRECISION_DEFAULT ${USE_BOOST_MULTIPRECISION_DEFAULT} CACHE INTERNAL "")
ELSE()
	SET(USE_BOOST_MULTIPRECISION_DEFAULT ON CACHE INTERNAL "")
ENDIF()

option(USE_BOOST_MULTIPRECISION "Can we use boost::multiprecision - it was broken https://svn.boost.org/trac/boost/ticket/12181" ${USE_BOOST_MULTIPRECISION_DEFAULT})

if(USE_BOOST_MULTIPRECISION)
	message(STATUS "USE_BOOST_MULTIPRECISION is normall (on)")
	add_definitions(-DUSE_BOOST_MULTIPRECISION=1)
else()
	message(STATUS "USE_BOOST_MULTIPRECISION is DISABLED")
	add_definitions(-DUSE_BOOST_MULTIPRECISION=0)
endif()

########################### SODIUM ###########################
find_package(Sodium)
link_directories(/home/ubuntu/build/libsodium/libsodium-osx/lib/) # for gitian build

if (NOT SODIUM_FOUND)
	message("\nSODIUM NOT FOUND!")
	if (MSVC)
		message(STATUS "\nNote (MSVC): \nPlease download and install package (or later):")
		message(STATUS "    https://download.libsodium.org/libsodium/releases/ libsodium-1.0.11-msvc.zip  \n")
		message(STATUS "Add following argument for cmake (install to location):")
		message(STATUS "    \"-DSODIUM_ROOT_DIR=C:/local/libsodium-1.0.11-msvc/\"\n\n")
	endif()
	message(FATAL_ERROR "SODIUM REQUIRED AND NOT FOUND, NOW STOPPING")
endif()

include_directories(SYSTEM ${SODIUM_INCLUDE_DIR})
#add local gtest
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/depends/googletest ${CMAKE_CURRENT_BINARY_DIR}/googletest)
include_directories(depends/googletest/googletest/include/)

#add local jsoncpp
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/depends/jsoncpp ${CMAKE_CURRENT_BINARY_DIR}/jsoncpp)
include_directories(depends/jsoncpp/include)
link_directories(depends/jsoncpp/src/lib_json)

#add local nlohmann json
include_directories(depends/json/src)

#add local CTPL
include_directories(depends/CTPL)

#add local backward-cpp for backtrace printing
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/depends/backward-cpp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/depends/backward-cpp)

#add local local lib sodiumpp
set (SODIUMPP_STATIC 1)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/depends/sodiumpp ${CMAKE_CURRENT_BINARY_DIR}/sodiumpp)
include_directories(depends/sodiumpp/sodiumpp/include)

########################### SIDH ###########################
if(ENABLE_CRYPTO_SIDH AND NOT MSVC)
	message(STATUS "Using crypto: SIDH")
	if(NOT EXTLEVEL_IS_EXPERIMENT_DANGEROUS)
    message( FATAL_ERROR "To use SIDH you must also enable proper EXTLEVEL for it (see the CMake file, or use defaults)" )
	endif()
	add_definitions(-DENABLE_CRYPTO_SIDH_CMAKE=1) # MATCH: must match also in project.hpp!

	#add local lib sidh
	#flags for SIDH.h
	add_definitions(-D_AMD64_ -D__LINUX__) #TODO set this flags automatically
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/depends/sidh)
	include_directories(depends/sidh)
	list(APPEND LIBS_OPTIONAL_CRYPTO "sidh")
else()
	add_definitions(-DENABLE_CRYPTO_SIDH_CMAKE=0) # MATCH: must match also in project.hpp!
endif()

########################### NTru ###########################
if(ENABLE_CRYPTO_NTRU AND NOT MSVC)
	message(STATUS "Using crypto: NTru")
	add_definitions(-DENABLE_CRYPTO_NTRU_CMAKE=1) # MATCH: must match also in project.hpp!

	set(NTRU_SIGN_DIR "depends/ntru-crypto/reference-code/C/Sign/PASS/src")
	include_directories("${NTRU_SIGN_DIR}")
	file(GLOB ntru-sign-files ${NTRU_SIGN_DIR}/*.c)
	file(GLOB BENCH ${NTRU_SIGN_DIR}/bench.c)
	list(REMOVE_ITEM ntru-sign-files ${BENCH})
	add_library(ntrusign ${ntru-sign-files})
	target_link_libraries(ntrusign fftw3 m)
	#add local build of some external libs
	link_directories( build_extra/ntru/.libs )
	list(APPEND LIBS_OPTIONAL_CRYPTO "ntruencrypt")
	list(APPEND LIBS_OPTIONAL_CRYPTO "ntrusign")

	message(STATUS "This is one part of NTRU (the sign), and there is other part of NTRU (encr) executed in other files (e.g. in scripts)")
  # NTRU Encrypt is in other file ---^
else()
	add_definitions(-DENABLE_CRYPTO_NTRU_CMAKE=0) # MATCH (name of this option) must match also in project.hpp!
endif()

# used e.g. by various crypto like ENABLE_CRYPTO_SIDH , ENABLE_CRYPTO_NTRU
include_directories( build_extra )


########################### COMPILER FLAGS ###########################

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(PEDANTIC_CXX_FLAGS "-Wall -Wextra -pedantic -Wno-missing-braces -Wcast-align -Wctor-dtor-privacy -Wdisabled-optimization -Winit-self -Wmissing-include-dirs -Wswitch-default -Wold-style-cast -Woverloaded-virtual -Wformat=2 -Wno-format-nonliteral ${GCC_WARN_FLAGS}")
if(${CMAKE_CXX_COMPILER_ID} MATCHES GNU)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
	set(WARN_PEDANTIC_FLAGS "-Wall -Wextra -pedantic -Wnoexcept -Wstrict-null-sentinel -Wlogical-op")
	string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
	string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
	if(APPLE)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
		set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -stdlib=libc++")
	endif()
	# https://clang.llvm.org/docs/ThreadSafetyAnalysis.html
	set(WARN_PEDANTIC_FLAGS "-Wall -Wextra -pedantic -Wno-unused-command-line-argument -Wthread-safety")
	string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
	string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES MSVC) #TODO check this condition
	set(WARN_PEDANTIC_FLAGS "/Wall")
	string(REPLACE "/DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
	string(REPLACE "/DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
endif()


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${STATIC_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARN_PEDANTIC_FLAGS} ${STATIC_FLAGS} ${COVERAGE_CXX_FLAG}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MINGW_LINK_FLAGS}")

# more directories of my main code

########################### SOURCE FILES ###########################
include_directories( src/ )

file(GLOB SOURCES_GROUP_CRYPTO src/crypto/*.cpp)

file(GLOB SOURCES_GROUP_TUNSERVER src/*.cpp src/httpdbg/peering_stats.cpp src/utils/* src/stdplus/* src/cable/* src/cable/*/* src/tuntap/base/* src/tuntap/linux/* src/tuntap/windows/* src/tuntap/macosx/*)

if(WIN32 OR CYGWIN OR MINGW)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/contrib/windows-service-wrapper)
	add_custom_target(windowsService)
	add_dependencies(windowsService galaxyService)
endif()

option(HTTP_DEBUGGER "Build with http debuger" OFF)
if(HTTP_DEBUGGER)
	add_definitions(-DHTTP_DBG)
endif()

list(REMOVE_ITEM SOURCES_GROUP_TUNSERVER ${CMAKE_CURRENT_SOURCE_DIR}/src/g42-main.cpp)
file(GLOB SOURCES_GROUP_RPC src/rpc/*.cpp)
list(REMOVE_ITEM SOURCES_GROUP_RPC ${CMAKE_CURRENT_SOURCE_DIR}/src/rpc_sender.cpp)

if(APPLE)
	list(APPEND SOURCES_GROUP_TUNSERVER ${CMAKE_CURRENT_SOURCE_DIR}/depends/cjdns-code/NetPlatform_darwin.c)
elseif(WIN32)

else()
	list(APPEND SOURCES_GROUP_TUNSERVER ${CMAKE_CURRENT_SOURCE_DIR}/depends/cjdns-code/NetPlatform_linux.c)
endif()

message(STATUS "Sorting sources (from GLOB)")
list(SORT SOURCES_GROUP_TUNSERVER)
list(SORT SOURCES_GROUP_CRYPTO)
list(SORT SOURCES_GROUP_RPC)
#message("Sorted sources are:" "${SOURCES_GROUP_TUNSERVER}")
#message("Sorted sources are:" "${SOURCES_GROUP_CRYPTO}")
#message("Sorted sources are:" "${SOURCES_GROUP_RPC}")

include_directories(../antinet/src/antinet_sim/)
# add_executable(ipclient.elf ipclient.cpp counter.cpp)
# add_executable(ipserver.elf ipserver.cpp)
add_executable(tunserver.elf src/g42-main.cpp )
cotire(tunserver.elf)


string(STRIP "${LIBS_OPTIONAL_CRYPTO}" LIBS_OPTIONAL_CRYPTO_clean)
message(STATUS "Libs for crypto are: (as string) '${LIBS_OPTIONAL_CRYPTO_clean}'")
separate_arguments(LIBS_OPTIONAL_CRYPTO_clean)
unset(LIBS_OPTIONAL_CRYPTO)
message(STATUS "Libs for crypto are: (as list) '${LIBS_OPTIONAL_CRYPTO_clean}'")

## TODO check this for gitian
# LIBS-1 [new doc]

# LIBS-2 [new doc]

#add_library(tunserver counter.cpp cjdns-code/NetPlatform_linux.c c_ip46_addr.cpp
#	c_peering.cpp strings_utils.cpp haship.cpp testcase.cpp protocol.cpp libs0.cpp filestorage.cpp ../antinet/src/antinet_sim/c_tnetdbg.cpp
#	trivialserialize.cpp glue_lockedstring_trivialserialize.cpp
#	generate_config.cpp text_ui.cpp c_json_load.cpp c_json_genconf.cpp galaxy_debug.cpp
#	rpc/rpc.cpp rpc/c_connection_base.cpp rpc/c_tcp_asio_node.cpp ${SOURCES_GROUP_CRYPTO})
add_library(tunserver ${SOURCES_GROUP_TUNSERVER} ${SOURCES_GROUP_CRYPTO} ${SOURCES_GROUP_RPC})
cotire(tunserver)


target_link_libraries(tunserver.elf tunserver ${Boost_LIBRARIES} ${BOOST_LOCALE_LINK_LIB} ${BOOST_THREAD_LINK_LIB}
	${PTHREAD_LIB} sodiumpp ${SODIUM_LIB} jsoncpp_lib_static ${LIBS_OPTIONAL_CRYPTO_clean} ${MINGW_LIBRARIES} ${MSVC_LIRARIES})
#tests
file(GLOB TEST_SOURCES "src/test/*.cpp")
list(SORT TEST_SOURCES)
#message("Sorted sources are:" "${TEST_SOURCES}")

#debug version
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
add_executable(test-debug.elf ${TEST_SOURCES})
target_link_libraries(test-debug.elf tunserver ${Boost_LIBRARIES} ${BOOST_LOCALE_LINK_LIB} ${BOOST_THREAD_LINK_LIB}
	gtest sodiumpp ${SODIUM_LIB} jsoncpp_lib_static ${MINGW_LIBRARIES})
foreach (_lib ${LIBS_OPTIONAL_CRYPTO_clean})
	message(STATUS "ADDING LIBRARY FROM LIST: '${_lib}'")
	target_link_libraries(test-debug.elf ${_lib})
endforeach()

#release version
set(CMAKE_CXX_FLAGS "-DRELEASEMODE=1")
add_executable(test-release.elf ${TEST_SOURCES})
target_link_libraries(test-release.elf tunserver ${Boost_LIBRARIES} ${BOOST_LOCALE_LINK_LIB} ${BOOST_THREAD_LINK_LIB}
	gtest sodiumpp ${SODIUM_LIB} jsoncpp_lib_static ${MINGW_LIBRARIES})
foreach (_lib ${LIBS_OPTIONAL_CRYPTO_clean})
	message(STATUS "ADDING LIBRARY FROM LIST: '${_lib}'")
	target_link_libraries(test-release.elf ${_lib})
endforeach()

# This will add libraries, definitions and include directories needed by backward
# by setting each property on the target.
add_backward(tunserver.elf)
add_backward(test-debug.elf)
add_backward(test-release.elf)

message(STATUS "C++ Compiler ID:               ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C Flags:                       ${CMAKE_C_FLAGS}")
message(STATUS "C++ Flags:                     ${CMAKE_CXX_FLAGS}")
message(STATUS "C++ link flags:                ${CMAKE_CXX_LINK_FLAGS}")
message(STATUS "Debug flags:                   ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "Release flags:                 ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "Release with debug info flags: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "Warn pedantic flags:           ${WARN_PEDANTIC_FLAGS}")

#file(GLOB SOURCES_GROUP_RPC rpc/*.cpp)
# ^-- if using this, then SORT it.
#add_executable(rpc_sender ${SOURCES_GROUP_RPC} ../antinet/src/antinet_sim/c_tnetdbg.cpp)
#target_link_libraries(rpc_sender boost_system pthread)

if(NOT MINGW AND NOT WIN32)
add_custom_target(run
		COMMAND "${CMAKE_SOURCE_DIR}/rundevel.sh"
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
	COMMAND cp "${CMAKE_SOURCE_DIR}/rundevel.sh" "./" || echo skip
	WORKING_DIRECTORY ./)

add_custom_command(
	TARGET ipclient.elf ipserver.elf tunserver.elf
	POST_BUILD
	COMMAND "${CMAKE_SOURCE_DIR}/share/script/use-setcap" tunserver.elf || echo "Can not set rights on binary, ignoring."
	WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

add_custom_target(
	TARGET ipclient.elf ipserver.elf tunserver.elf
	POST_BUILD
	COMMAND "${CMAKE_SOURCE_DIR}/share/script/show-source-help.sh"
	COMMAND "${CMAKE_SOURCE_DIR}/share/script/show-program-help.sh"
	WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

endif()
