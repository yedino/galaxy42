cmake_minimum_required(VERSION 3.0)
project (galaxy42)

# IMPORTANT:
# search in ALL other files (source code, the bash scripts ./do and others)
# following topics to match this topics here:
#
# match_extlevel_* - this are places that decide on which EXTLEVEL given feature is enabled:
# {{match_extlevel_sidh}} - SIDH crypto lib: now at level 30 (EXTLEVEL_IS_EXPERIMENT_DANGEROUS)
# {{match_extlevel_ntru}} - NTRU crypto lib (encr, and sign): now at level 20 (EXTLEVEL_IS_EXPERIMENT)
#

if (NOT CMAKE_BUILD_TYPE)
	message(STATUS "No build type selected, default to Debug")
	set(CMAKE_BUILD_TYPE "Debug")
endif()

########################### CMAKE MODULES ###########################
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/depends/cotire/CMake")
include(cotire)

if(MINGW)
	add_definitions(-D_GLIBCXX_USE_CXX11_ABI=1)
#	include_directories(depends/gettext/gettext-0.19.8/install_dir/include/)
#	link_directories(depends/gettext/gettext-0.19.8/install_dir/lib)
#	link_directories(depends/gettext/libiconv-1.14/install_dir/lib)
	set(MINGW_LINK_FLAGS "-Wl,--allow-multiple-definition")
	set(MINGW_LIBRARIES ws2_32 iphlpapi mswsock)
endif()

if(WIN32 OR CYGWIN)
	add_definitions(-DUNICODE)
	add_definitions(-D_UNICODE)
	add_definitions(-DWINVER=0x0601)
	add_definitions(-D_WIN32_WINNT=0x0601)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

########################### GENERAL ############################
set(TOOLS_ARE_BROKEN OFF) # <-- set this flag to ON if we detect any problems, e.g. that user has broken libraries
set(TOOLS_ARE_BROKEN_UB OFF) # <-- do we have some tools (e.g. libs) that include UBs (this means e.g. we can't use UBSAN correctly)

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

######################################################################
########################### LIBS CHECK - FOR FLAGS ###################
######################################################################
# Quick check of libraries and dependencies,
# to decide on flags to use.
# (E.g. some buggy libraries will cause us to remove UBSAN or some other checks)


########################### BOOST ###########################
### boost - part 1 - just check version and decide does it affect our flags
set (Boost_USE_MULTITHREADED ON)
if (BUILD_STATIC_TUNSERVER)
	set (Boost_USE_STATIC_RUNTIME ON)
	set (Boost_USE_STATIC_LIBS ON)
	set (Boost_USE_DEBUG_RUNTIME OFF)
	set (sodium_USE_STATIC_LIBS ON)
endif()

if (MSVC)
	set(BOOST_LIBS_LIST system filesystem thread date_time)
elseif (MINGW)
	set(BOOST_LIBS_LIST system filesystem program_options date_time)
else()
	set(BOOST_LIBS_LIST system filesystem program_options thread date_time)
endif()
find_package(Boost 1.49.0 REQUIRED COMPONENTS ${BOOST_LIBS_LIST})

if(NOT Boost_FOUND)
	message("\nBOOST NOT FOUND!")
	if (MSVC)
		message(STATUS "\nNote (MSVC): \nPlease download and install package (or later):")
		message(STATUS "    https://sourceforge.net/projects/boost/files/boost-binaries/ boost_1_61_0-msvc-14.0-64.exe \n")
		message(STATUS "Add following argument for cmake (default install location):")
		message(STATUS "    \"-DBOOST_ROOT=C:/local/boost_1_61_0 -DBOOST_LIBRARYDIR=C:/local/boost_1_61_0/lib64-msvc-14.0/\"\n\n")
	endif()
	message(FATAL_ERROR "BOOST REQUIRED AND NOT FOUND, NOW STOPPING")
endif()

message(STATUS "Boost version variable is: ${Boost_VERSION}")
set(Boost_IS_BROKEN_UB OFF)
if ((Boost_VERSION GREATER 105399) AND (Boost_VERSION LESS 105700)) # 1.54 is broken, fix in 1.57
	set(Boost_IS_BROKEN_UB ON)
	message(WARNING "Warning: this Boost version is known to have a bug: https://svn.boost.org/trac10/ticket/12570")
endif()

if (Boost_IS_BROKEN_UB)
	set(TOOLS_ARE_BROKEN ON)
	set(TOOLS_ARE_BROKEN_UB ON)
	message(WARNING "Warning: this Boost version has bug(s) - that will theoretically cause C++ UB (bug in our program), use other one if possible, we can not guarantee security of program built here!")
endif()

######################################################################
########################### COMPILER FLAGS ###########################
######################################################################

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (CMAKE_VERSION VERSION_LESS 3.1)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --std=c++${CMAKE_CXX_STANDARD}") # [[version]] needed for Debian 8
endif()

if(${CMAKE_CXX_COMPILER_ID} MATCHES GNU)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
	set(WARN_PEDANTIC_FLAGS "-Wall -Wextra -pedantic -Wnoexcept -Wstrict-null-sentinel -Wlogical-op")
	set(WARN_PEDANTIC_FLAGS "${WARN_PEDANTIC_FLAGS} -Wreturn-type")
	set(WARN_PEDANTIC_FLAGS_FOR_C "-Wall") # mostly used by dependencies
	string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
	string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
elseif(${CMAKE_CXX_COMPILER_ID} MATCHES Clang)
	if(APPLE)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++")
		set(CMAKE_CXX_LINK_FLAGS "${CMAKE_CXX_LINK_FLAGS} -stdlib=libc++")
	endif()

	### clang sanitizers ###
	option(SANITIZER_THREAD "Enable TSAN - clang thread sanitizer" OFF)
	option(SANITIZER_ADDRESS "Enable ASAN - clang address sanitizer" OFF)
	option(SANITIZER_UNDEFINED_BEHAVIOR "Enable UBSAN - clang undefined behavior sanitizer" OFF)
	set(SANITIZER_DETAILED "-fno-omit-frame-pointer -fno-optimize-sibling-calls")
	if(SANITIZER_THREAD)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=thread -fno-sanitize-recover=thread ${SANITIZER_DETAILED}")
		add_definitions(-DFLAVOUR_TSAN_FULL) # we have full TSAN and can rely on it
	endif()
	if(SANITIZER_ADDRESS)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-sanitize-recover=address ${SANITIZER_DETAILED}")
	endif()
	if(SANITIZER_UNDEFINED_BEHAVIOR)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=undefined -fsanitize=integer ${SANITIZER_DETAILED}")
		if (NOT TOOLS_ARE_BROKEN_UB)
			set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-sanitize-recover=undefined")
			add_definitions(-DFLAVOUR_UBSAN_FULL) # we have full UBSAN and can rely on it
		else()
			message(WARNING "Warning: we have to partially disable UBSAN check (that is: program will continue to run, ignoring an UB!) because the tools used are broken and produce UBs.")
			add_definitions(-DFLAVOUR_UBSAN_ONLY_REPORTS) # we have partiall UBSAN and can not rely on it's results
		endif()
	endif()
	# https://clang.llvm.org/docs/ThreadSafetyAnalysis.html
	set(WARN_PEDANTIC_FLAGS "-Wall -Wextra -pedantic -Wno-unused-command-line-argument -Wthread-safety")

	set(WARN_PEDANTIC_FLAGS "${WARN_PEDANTIC_FLAGS} -Wreturn-type")
	set(WARN_PEDANTIC_FLAGS_FOR_C "-Wall -Wno-unused-command-line-argument") # mostly used by dependencies
	string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
	string(REPLACE "-DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
	set(WARN_PEDANTIC_FLAGS "/W4")
	string(REPLACE "/DNDEBUG" "" CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")
	string(REPLACE "/DNDEBUG" "" CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
else()
	message(fatal_error "Compiler not supported")
endif()


set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARN_PEDANTIC_FLAGS_FOR_C} ${STATIC_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARN_PEDANTIC_FLAGS} ${STATIC_FLAGS} ${COVERAGE_CXX_FLAG}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${MINGW_LINK_FLAGS}")

# more directories of my main code

######################################################################
########################### DEPENDS ##################################
######################################################################

########################### BOOST ###########################
### boost - part2
include_directories(SYSTEM ${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARY_DIRS})

IF(DEFINED USE_BOOST_MULTIPRECISION_DEFAULT)
	SET(USE_BOOST_MULTIPRECISION_DEFAULT ${USE_BOOST_MULTIPRECISION_DEFAULT} CACHE INTERNAL "")
ELSE()
	SET(USE_BOOST_MULTIPRECISION_DEFAULT ON CACHE INTERNAL "")
ENDIF()

option(USE_BOOST_MULTIPRECISION "Can we use boost::multiprecision - it was broken https://svn.boost.org/trac/boost/ticket/12181" ${USE_BOOST_MULTIPRECISION_DEFAULT})

if(USE_BOOST_MULTIPRECISION)
	message(STATUS "USE_BOOST_MULTIPRECISION is normal (on)")
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
if(CYGWIN)
	set(SODIUM_LIB "sodium")
elseif(MSVC OR MINGW)
	set(SODIUM_LIB "${SODIUM_LIBRARY}")
	set(MSVC_LIRARIES "iphlpapi")
else()
	set(PTHREAD_LIB "pthread")
	set(SODIUM_LIB "sodium")
endif()

#add local gtest
if (MSVC)
	set(gtest_force_shared_crt ON CACHE BOOL "Enable gtest_force_shared_crt")
endif()
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/depends/googletest ${CMAKE_CURRENT_BINARY_DIR}/googletest)
include_directories(depends/googletest/googletest/include/)
include_directories(depends/googletest/googlemock/include/)

#add local nlohmann json
include_directories(depends/json/src)

#add local CTPL
include_directories(depends/CTPL)

#add local backward-cpp for backtrace printing
set(HAVE_BACKWARD 0)
if(NOT WIN32)
	set(Backward_DIR ${CMAKE_BINARY_DIR}/depends/backward-cpp)
	find_package(Backward)
	if(Backward_FOUND)
		set(HAVE_BACKWARD 1)
	endif()
endif()
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


########################### SOURCE FILES ###########################
include_directories( src/ )
include_directories( src-tools/netmodel/ )

file(GLOB SOURCES_GROUP_CRYPTO src/crypto/*.cpp)

file(GLOB SOURCES_GROUP_TUNSERVER src/* src/httpdbg/* src/utils/* src/stdplus/* src/cable/* src/cable/base/* src/cable/simulation/* src/cable/udp/* src/cable/shm/cable_shm_addr.cpp *src/tuntap/base/* src/tuntap/linux/* src/tuntap/windows/* src/tuntap/macosx/*
	src/test/special_behaviour/*
)
list(REMOVE_ITEM SOURCES_GROUP_TUNSERVER
	${CMAKE_CURRENT_SOURCE_DIR}/src/ipserver.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/ipclient.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/style_example.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/src/g42-main.cpp)

if(APPLE)
	list(APPEND SOURCES_GROUP_TUNSERVER ${CMAKE_CURRENT_SOURCE_DIR}/depends/cjdns-code/NetPlatform_darwin.c)
elseif(NOT WIN32)
	list(APPEND SOURCES_GROUP_TUNSERVER ${CMAKE_CURRENT_SOURCE_DIR}/depends/cjdns-code/NetPlatform_linux.c)
endif()

file(GLOB SOURCES_GROUP_TOOLS_NETMODEL src-tools/netmodel/*.cpp src-tools/netmodel/*/*.cpp
)

if(WIN32 OR CYGWIN OR MINGW)
	add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/contrib/windows-service-wrapper)
	add_custom_target(windowsService)
	add_dependencies(windowsService galaxyService)
endif()

option(HTTP_DEBUGGER "Build with http debuger" OFF)
if(HTTP_DEBUGGER)
	add_definitions(-DHTTP_DBG)
endif()

file(GLOB SOURCES_GROUP_RPC src/rpc/*.cpp)
list(REMOVE_ITEM SOURCES_GROUP_RPC ${CMAKE_CURRENT_SOURCE_DIR}/src/rpc_sender.cpp)

file(GLOB SOURCES_API_GALAXY src/apigalaxy/*)

message(STATUS "Sorting sources (from GLOB) - must be sorted for Gitian to be deterministic")
list(SORT SOURCES_GROUP_TUNSERVER)
list(SORT SOURCES_GROUP_TOOLS_NETMODEL)
list(SORT SOURCES_GROUP_CRYPTO)
list(SORT SOURCES_GROUP_RPC)
list(SORT SOURCES_API_GALAXY)

include_directories(../antinet/src/antinet_sim/)


string(STRIP "${LIBS_OPTIONAL_CRYPTO}" LIBS_OPTIONAL_CRYPTO_clean)
message(STATUS "Libs for crypto are: (as string) '${LIBS_OPTIONAL_CRYPTO_clean}'")
separate_arguments(LIBS_OPTIONAL_CRYPTO_clean)
unset(LIBS_OPTIONAL_CRYPTO)
message(STATUS "Libs for crypto are: (as list) '${LIBS_OPTIONAL_CRYPTO_clean}'")

## TODO check this for gitian
# LIBS-1 [new doc]

# LIBS-2 [new doc]


########################### OPTIONAL LIBRARIES ###########################

set(LIB_OPTIONAL_CAPNG "")
if (UNIX AND NOT CYGWIN AND NOT APPLE)
	set(LIB_OPTIONAL_CAPNG "cap-ng")
endif ()

######################################################################
######################################################################
######################################################################

########################### TARGETS ###########################
add_library(apigalaxy ${SOURCES_API_GALAXY})

add_executable(tunserver.elf src/g42-main.cpp ${SOURCES_GROUP_TUNSERVER} ${SOURCES_GROUP_TOOLS_NETMODEL} ${SOURCES_GROUP_CRYPTO} ${SOURCES_GROUP_RPC})
if (NOT MSVC) #TODO
	cotire(tunserver.elf)
endif()

set(LIBS_COMMON
	${Boost_LIBRARIES} ${BOOST_LOCALE_LINK_LIB} ${BOOST_THREAD_LINK_LIB}
	${PTHREAD_LIB} sodiumpp ${SODIUM_LIB} ${LIBS_OPTIONAL_CRYPTO_clean}
	${LIB_OPTIONAL_CAPNG}
	${MINGW_LIBRARIES} ${MSVC_LIRARIES} apigalaxy
)

target_link_libraries(tunserver.elf
	${LIBS_COMMON}
	)
# TODO remove when confirmed that above LIBS_COMMON do work correctly instead (if no problem till max 2017-07-01)
#	${Boost_LIBRARIES} ${BOOST_LOCALE_LINK_LIB} ${BOOST_THREAD_LINK_LIB}
#	${PTHREAD_LIB} sodiumpp ${SODIUM_LIB} ${LIBS_OPTIONAL_CRYPTO_clean}
#	${MINGW_LIBRARIES} ${MSVC_LIRARIES})

#tests
file(GLOB TEST_SOURCES "src/test/*.cpp")
list(SORT TEST_SOURCES)

add_executable(unittests.elf ${SOURCES_GROUP_TUNSERVER} ${SOURCES_GROUP_TOOLS_NETMODEL} ${SOURCES_GROUP_CRYPTO} ${SOURCES_GROUP_RPC} ${TEST_SOURCES})
target_compile_definitions(unittests.elf PUBLIC -DUSE_MOCK)
target_link_libraries(unittests.elf
	${LIBS_COMMON}
	gtest gmock
)

#	${Boost_LIBRARIES} ${BOOST_LOCALE_LINK_LIB} ${BOOST_THREAD_LINK_LIB}
#	gtest gmock sodiumpp ${SODIUM_LIB} ${MINGW_LIBRARIES})
#foreach (_lib ${LIBS_OPTIONAL_CRYPTO_clean})
#	message(STATUS "ADDING LIBRARY FROM LIST: '${_lib}'")
#	target_link_libraries(test-debug.elf ${_lib})
#endforeach()

# This will add libraries, definitions and include directories needed by backward
# by setting each property on the target.
add_executable(performancetests.elf src/test/performance/xint.cpp ${SOURCES_GROUP_TUNSERVER} ${SOURCES_GROUP_TOOLS_NETMODEL} ${SOURCES_GROUP_CRYPTO} ${SOURCES_GROUP_RPC})
target_link_libraries(performancetests.elf
	${LIBS_COMMON})

if(HAVE_BACKWARD AND NOT MINGW)
#	add_backward(tunserver)
	add_backward(tunserver.elf)
	add_backward(unittests.elf)
endif()

########################### FINALIZE OPTIONS #########################

if (TOOLS_ARE_BROKEN)
	# pass on to the C++ option informing that tools are broken
	add_definitions(-DTOOLS_ARE_BROKEN=1)
endif()

message(STATUS "C++ Compiler ID:               ${CMAKE_CXX_COMPILER_ID}")
message(STATUS "C Flags:                       ${CMAKE_C_FLAGS}")
message(STATUS "C++ Flags:                     ${CMAKE_CXX_FLAGS}")
message(STATUS "C++ link flags:                ${CMAKE_CXX_LINK_FLAGS}")
message(STATUS "Build type:                    ${CMAKE_BUILD_TYPE}")
message(STATUS "Debug flags:                   ${CMAKE_CXX_FLAGS_DEBUG}")
message(STATUS "Release flags:                 ${CMAKE_CXX_FLAGS_RELEASE}")
message(STATUS "Release with debug info flags: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
message(STATUS "Warn pedantic flags:           ${WARN_PEDANTIC_FLAGS}")

########################### CUSTOM TARGETS ###########################
add_custom_target(Release
		COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Release ${CMAKE_SOURCE_DIR}
		COMMAND make)
add_custom_target(Debug
		COMMAND ${CMAKE_COMMAND} -DCMAKE_BUILD_TYPE=Debug ${CMAKE_SOURCE_DIR}
		COMMAND make)

if(NOT MINGW AND NOT WIN32)

	add_custom_target(run
			COMMAND "${CMAKE_BINARY_DIR}/rundevel.sh"
			DEPENDS ./tunserver.elf
			DEPENDS unittests.elf
			WORKING_DIRECTORY ./)

	add_custom_command(
		TARGET tunserver.elf
		POST_BUILD
		COMMAND rm -f "./nocap-tunserver.elf"
		COMMAND chmod g-wx,o-wx tunserver.elf
		COMMAND cp tunserver.elf nocap-tunserver.elf
		# remove and copy again scripts: run
		COMMAND rm -f "./rundevel.sh"
		COMMAND cp "${CMAKE_SOURCE_DIR}/src/scripts/run/rundevel.sh" "${CMAKE_BINARY_DIR}/" || echo "Can not copy file"
		# remove and copy again scripts: qa/run-safe
		# remove here all files copied [[list_qa_run_safe_files]]
		COMMAND rm -f "./run-safe-mem"
		COMMAND rm -f "./run-safe-thread-ub"
		COMMAND rm -f "./valgrind_log_watch"
		COMMAND rm -f "./valgrind_with_log_kill"
		COMMAND cp "${CMAKE_SOURCE_DIR}/qa/run-safe/*" "${CMAKE_BINARY_DIR}/" || echo skip

		WORKING_DIRECTORY ./)

	add_custom_command(
		TARGET tunserver.elf
		POST_BUILD
		COMMAND "${CMAKE_SOURCE_DIR}/share/script/use-setcap" tunserver.elf || echo "Can not set rights cap on binary, ignoring."
		COMMAND chmod a+x tunserver.elf
		COMMAND chmod a+x nocap-tunserver.elf
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

	add_custom_command(
		TARGET unittests.elf
		POST_BUILD
		COMMAND chmod g-wx,o-wx unittests.elf
		COMMAND cp unittests.elf nocap-unittests.elf
		COMMAND "${CMAKE_SOURCE_DIR}/share/script/use-setcap" unittests.elf || echo "Can not set rights cap on binary, ignoring."
		COMMAND chmod a+x unittests.elf
		COMMAND chmod a+x nocap-unittests.elf
		WORKING_DIRECTORY ${CMAKE_BINARY_DIR})

	add_custom_target(
		TARGET tunserver.elf
		POST_BUILD
		COMMAND "${CMAKE_SOURCE_DIR}/share/script/show-source-help.sh"
		COMMAND "${CMAKE_SOURCE_DIR}/share/script/show-program-help.sh"
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})
endif()
