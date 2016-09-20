# from: https://github.com/kostko/libcurvecpr-asio/blob/master/cmake/FindSodium.cmake
# aa358c98dec8fd0f31e7bab7a585571d5f8bfae3
#
# - Find Sodium
# Find the native libsodium includes and library.
# Once done this will define
#
#  SODIUM_INCLUDE_DIR    - where to find libsodium header files, etc.
#  SODIUM_LIBRARY        - List of libraries when using libsodium.
#  SODIUM_FOUND          - True if libsodium found.
#

# (from FindBoost script)
# Collect environment variable inputs as hints.  Do not consider changes.
foreach(v SODIUM_ROOT_DIR)
  set(_env $ENV{${v}})
  if(_env)
    file(TO_CMAKE_PATH "${_env}" _ENV_${v})
  else()
    set(_ENV_${v} "")
  endif()
endforeach()

FIND_LIBRARY(SODIUM_LIBRARY NAMES sodium libsodium
	HINTS
	${_ENV_SODIUM_ROOT_DIR}/lib
	${_ENV_SODIUM_ROOT_DIR}/Win32/Debug/v140/dynamic #TODO detect build type
	${_ENV_SODIUM_ROOT_DIR}/bin/Win32/Debug/v140/dynamic) #TODO detect build type

find_path(SODIUM_INCLUDE_DIR NAMES sodium.h
	HINTS
		${_ENV_SODIUM_ROOT_DIR}/include
		${_ENV_SODIUM_ROOT_DIR}/src/libsodium/include)

# handle the QUIETLY and REQUIRED arguments and set SODIUM_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Sodium REQUIRED_VARS SODIUM_LIBRARY SODIUM_INCLUDE_DIR)

MARK_AS_ADVANCED(SODIUM_LIBRARY SODIUM_INCLUDE_DIR)


