cmake_minimum_required(VERSION 2.8.3)
project (galaxy42)

# IMPORTANT:
# search in ALL other files (source code, the bash scripts ./do and others)
# following topics to match this topics here:
#
# match_extlevel_* - this are places that decide on which EXTLEVEL given feature is enabled:
# {{match_extlevel_sidh}} - SIDH crypto lib: now at level 30 (EXTLEVEL_IS_EXPERIMENT_DANGEROUS)
# {{match_extlevel_ntru}} - NTRU crypto lib (encr, and sign): now at level 20 (EXTLEVEL_IS_EXPERIMENT)
#

if(MINGW)
	if(CYGWIN)
		add_definitions(-D_GLIBCXX_USE_CXX11_ABI=0)
	else()
		add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)
	endif()
endif()

if (MSVC)
	if(NOT MSVC14)
		message(FATAL_ERROR "MSVC version not supported, try MSVC 14")
	endif()
endif()

if (MINGW)
	add_definitions(-D_WIN32_WINNT=0x0600)
endif()

set (Boost_DEBUG OFF)
if (NOT MSVC)
	set(BOOST_LIBS_LIST system filesystem program_options)
else()
	set(BOOST_LIBS_LIST system filesystem)
endif()
find_package(Boost 1.49.0 REQUIRED COMPONENTS ${BOOST_LIBS_LIST})

if (NOT Boost_FOUND)
message("\nBOOST NOT FOUND!")
	if (MSVC)
		message("\nNote (windows): \nPlease download and install package (or later):")
		message("    https://sourceforge.net/projects/boost/files/boost-binaries/ boost_1_61_0-msvc-14.0-64.exe \n")
		message("Use following argument for cmake (default install location):")
		message("    \"-DBOOST_ROOT=C:/local/boost_1_61_0 -DBOOST_LIBRARYDIR=C:/local/boost_1_61_0/lib64-msvc-14.0/\"\n\n")
	endif()
message(FATAL_ERROR "BOOST REQUIRED AND NOT FOUND, NOW STOPPING")
endif()

include_directories(/home/ubuntu/gettext/gettext-0.19.8.1/install_dir/include)
link_directories(/home/ubuntu/gettext/gettext-0.19.8.1/install_dir/lib)
link_directories(/home/ubuntu/gettext/libiconv-1.14/install_dir/lib)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(Sodium REQUIRED)

include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${SODIUM_INCLUDE_DIR})
link_directories(${Boost_LIBRARY_DIRS})

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

# move bash ENV $variables to CMake set

IF(DEFINED EXTLEVEL)
   SET(EXTLEVEL ${EXTLEVEL} CACHE STRING "")
ELSE()
   SET(EXTLEVEL 0 CACHE STRING "")
ENDIF()

IF(DEFINED COVERAGE)
   SET(COVERAGE ${COVERAGE} CACHE BOOL "")
ELSE()
   SET(COVERAGE OFF CACHE BOOL "")
ENDIF()

IF(DEFINED USE_BOOST_MULTIPRECISION_DEFAULT)
   SET(USE_BOOST_MULTIPRECISION_DEFAULT ${USE_BOOST_MULTIPRECISION_DEFAULT} CACHE INTERNAL "")
ELSE()
   SET(USE_BOOST_MULTIPRECISION_DEFAULT ON CACHE INTERNAL "")
ENDIF()

message("CMAKE_BUILD_TYPE = ${CMAKE_BUILD_TYPE}")
message("EXTLEVEL = ${EXTLEVEL}")
message("COVERAGE = ${COVERAGE}")
message("USE_BOOST_MULTIPRECISION_DEFAULT = ${USE_BOOST_MULTIPRECISION_DEFAULT}")

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

option(USE_BOOST_MULTIPRECISION "Can we use boost::multiprecision - it was broken https://svn.boost.org/trac/boost/ticket/12181" ${USE_BOOST_MULTIPRECISION_DEFAULT})

if(USE_BOOST_MULTIPRECISION)
	message("USE_BOOST_MULTIPRECISION is normall (on)")
	add_definitions(-DUSE_BOOST_MULTIPRECISION=1)
else()
	message("USE_BOOST_MULTIPRECISION is DISABLED")
	add_definitions(-DUSE_BOOST_MULTIPRECISION=0)
endif()

if(EXTLEVEL_IS_NORMAL)
	message("EXTLEVEL enabling EXTLEVEL_IS_NORMAL")
	add_definitions(-DEXTLEVEL_IS_NORMAL_CMAKE=1)
else()
	add_definitions(-DEXTLEVEL_IS_NORMAL_CMAKE=0)
endif()
if(EXTLEVEL_IS_PREVIEW)
	message("EXTLEVEL enabling EXTLEVEL_IS_PREVIEW")
	add_definitions(-DEXTLEVEL_IS_PREVIEW_CMAKE=1)
else()
	add_definitions(-DEXTLEVEL_IS_PREVIEW_CMAKE=0)
endif()
if(EXTLEVEL_IS_EXPERIMENT)
	message("EXTLEVEL enabling EXTLEVEL_IS_EXPERIMENT !!")
	add_definitions(-DEXTLEVEL_IS_EXPERIMENT_CMAKE=1)
else()
	add_definitions(-DEXTLEVEL_IS_EXPERIMENT_CMAKE=0)
endif()
if(EXTLEVEL_IS_EXPERIMENT_DANGEROUS)
	message("EXTLEVEL enabling EXTLEVEL_IS_EXPERIMENT !!! this code will be insecure!!!")
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


#add local gtest
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/depends/googletest ${CMAKE_CURRENT_BINARY_DIR}/googletest)
include_directories(depends/googletest/googletest/include/)

#add local jsoncpp
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/depends/jsoncpp ${CMAKE_CURRENT_BINARY_DIR}/jsoncpp)
include_directories(depends/jsoncpp/include)
link_directories(depends/jsoncpp/src/lib_json)

#add local local lib sodiumpp
set (SODIUMPP_STATIC 1)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/depends/sodiumpp ${CMAKE_CURRENT_BINARY_DIR}/sodiumpp)
include_directories(depends/sodiumpp/sodiumpp/include)

if(ENABLE_CRYPTO_SIDH)
	message("Using crypto: SIDH")
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

if(ENABLE_CRYPTO_NTRU)
	message("Using crypto: NTru")
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

	message("This is one part of NTRU (the sign), and there is other part of NTRU (encr) executed in other files (e.g. in scripts)")
  # NTRU Encrypt is in other file ---^
else()
	add_definitions(-DENABLE_CRYPTO_NTRU_CMAKE=0) # MATCH (name of this option) must match also in project.hpp!
endif()


# used e.g. by various crypto like ENABLE_CRYPTO_SIDH , ENABLE_CRYPTO_NTRU
include_directories( build_extra )

if(COVERAGE)
	message("Enabling COVERAGE")
	set(flag_coverage "--coverage")
else()
	message("Disabling COVERAGE")
	set(flag_coverage "")
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -pedantic ${flag_coverage}")

set(CMAKE_CXX_BASE_FLAGS "${CMAKE_CXX_FLAGS}")


ADD_CUSTOM_TARGET(Fast
		COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Fast ${CMAKE_SOURCE_DIR}
		COMMAND make)
ADD_CUSTOM_TARGET(Release
		COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
		COMMAND make)
ADD_CUSTOM_TARGET(Debug
		COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
		COMMAND make)


# more directories of my main code

file(GLOB SOURCES_GROUP_CRYPTO src/crypto/*.cpp)
file(GLOB SOURCES_GROUP_TUNSERVER src/*.cpp src/crypto-sodium/*.cpp)
list(REMOVE_ITEM SOURCES_GROUP_TUNSERVER ${CMAKE_CURRENT_SOURCE_DIR}/src/g42-main.cpp)
file(GLOB SOURCES_GROUP_RPC src/rpc/*.cpp)
list(REMOVE_ITEM SOURCES_GROUP_RPC ${CMAKE_CURRENT_SOURCE_DIR}/src/rpc_sender.cpp)

if(APPLE)
	add_compile_options(-Wextra -Wno-unused-command-line-argument)	# unrecognized compilation flags on msvc
elseif(WIN32)

else()
	list(APPEND SOURCES_GROUP_TUNSERVER ${CMAKE_CURRENT_SOURCE_DIR}/depends/cjdns-code/NetPlatform_linux.c)
	add_compile_options(-Wextra -Wno-unused-command-line-argument)	# unrecognized compilation flags on msvc
endif()

message("Sorting sources (from GLOB)")
list(SORT SOURCES_GROUP_TUNSERVER)
list(SORT SOURCES_GROUP_CRYPTO)
list(SORT SOURCES_GROUP_RPC)
message("Sorted sources are:" "${SOURCES_GROUP_TUNSERVER}")
message("Sorted sources are:" "${SOURCES_GROUP_CRYPTO}")
message("Sorted sources are:" "${SOURCES_GROUP_RPC}")

set(CMAKE_CXX_FAST_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O2 -Wno-unused-parameter -Wno-unused-variable")


if(CMAKE_BUILD_TYPE STREQUAL "Fast")
        message("Fast build")
        add_definitions(-DRELEASEMODE_ -DNDEBUG)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FAST_FLAGS} -g0 -Ofast")

	elseif(CMAKE_BUILD_TYPE STREQUAL "Debug")
        message("Debug build")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O0")

	elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
        message("Release build")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -O2 -DNDEBUG -DRELEASEMODE")
else()
    message("bad CMAKE_BUILD_TYPE flag")
    message("usage is: cmake [Fast/Debug/Release] .")
    message( FATAL_ERROR "error" )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FAST_FLAGS}")
endif()

include_directories(../antinet/src/antinet_sim/)
# add_executable(ipclient.elf ipclient.cpp counter.cpp)
# add_executable(ipserver.elf ipserver.cpp)
add_executable(tunserver.elf src/g42-main.cpp )

if(WIN32 OR CYGWIN)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/contrib/windows-service-wrapper)
	add_custom_target(windowsService)
	add_dependencies(windowsService galaxyService)
endif()

string(STRIP "${LIBS_OPTIONAL_CRYPTO}" LIBS_OPTIONAL_CRYPTO_clean)
message("Libs for crypto are: (as string) '${LIBS_OPTIONAL_CRYPTO_clean}'")
separate_arguments(LIBS_OPTIONAL_CRYPTO_clean)
unset(LIBS_OPTIONAL_CRYPTO)
message("Libs for crypto are: (as list) '${LIBS_OPTIONAL_CRYPTO_clean}'")

## TODO check this for gitian
# LIBS-1 [new doc]

# LIBS-2 [new doc]

#add_library(tunserver counter.cpp cjdns-code/NetPlatform_linux.c c_ip46_addr.cpp
#	c_peering.cpp strings_utils.cpp haship.cpp testcase.cpp protocol.cpp libs0.cpp filestorage.cpp ../antinet/src/antinet_sim/c_tnetdbg.cpp
#	trivialserialize.cpp glue_lockedstring_trivialserialize.cpp
#	generate_config.cpp text_ui.cpp c_json_load.cpp c_json_genconf.cpp galaxy_debug.cpp
#	rpc/rpc.cpp rpc/c_connection_base.cpp rpc/c_tcp_asio_node.cpp ${SOURCES_GROUP_CRYPTO})
add_library(tunserver ${SOURCES_GROUP_TUNSERVER} ${SOURCES_GROUP_CRYPTO} ${SOURCES_GROUP_RPC})


if(MINGW)
	set(MINGW_LIBRARIES ws2_32 iphlpapi)
endif()

if(NOT MSVC AND NOT MINGW)
	set(PTHREAD_LIB "pthread")
	set(SODIUM_LIB "sodium")
else()
	set(SODIUM_LIB "${SODIUM_LIBRARY}")
	set(MSVC_LIRARIES "iphlpapi")
endif()

target_link_libraries(tunserver.elf tunserver ${Boost_LIBRARIES}
	${PTHREAD_LIB} sodiumpp ${SODIUM_LIB} jsoncpp_lib_static ${LIBS_OPTIONAL_CRYPTO_clean} ${MINGW_LIBRARIES} ${MSVC_LIRARIES} intl pthread iconv)

#tests
file(GLOB TEST_SOURCES "src/test/*.cpp")
list(SORT TEST_SOURCES)
message("Sorted sources are:" "${TEST_SOURCES}")

#debug version
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
add_executable(test-debug.elf ${TEST_SOURCES})
target_link_libraries(test-debug.elf tunserver ${Boost_LIBRARIES}
	gtest ${SODIUM_LIB} sodiumpp jsoncpp_lib_static)
foreach (_lib ${LIBS_OPTIONAL_CRYPTO_clean})
	message("ADDING LIBRARY FROM LIST: '${_lib}'")
	target_link_libraries(test-debug.elf ${_lib})
endforeach()


#release version
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_BASE_FLAGS} -DRELEASEMODE=1")
add_executable(test-release.elf ${TEST_SOURCES})
target_link_libraries(test-release.elf tunserver ${Boost_LIBRARIES}
	gtest ${SODIUM_LIB} sodiumpp jsoncpp_lib_static)
foreach (_lib ${LIBS_OPTIONAL_CRYPTO_clean})
	message("ADDING LIBRARY FROM LIST: '${_lib}'")
	target_link_libraries(test-release.elf ${_lib})
endforeach()

#file(GLOB SOURCES_GROUP_RPC rpc/*.cpp)
# ^-- if using this, then SORT it.
#add_executable(rpc_sender ${SOURCES_GROUP_RPC} ../antinet/src/antinet_sim/c_tnetdbg.cpp)
#target_link_libraries(rpc_sender boost_system pthread)

if(NOT MINGW AND NOT WIN32)
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
	COMMAND share/script/use-setcap tunserver.elf || echo "Can not set rights on binary, ignoring."
	WORKING_DIRECTORY ./)
endif()

# unset( USE_BOOST_MULTIPRECISION_DEFAULT )
