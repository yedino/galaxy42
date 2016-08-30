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

FIND_LIBRARY(SODIUM_LIBRARY NAMES sodium libsodium HINTS ${SODIUM_ROOT_DIR}/lib)
find_path(SODIUM_INCLUDE_DIR NAMES sodium.h HINTS ${SODIUM_ROOT_DIR}/include)

# handle the QUIETLY and REQUIRED arguments and set SODIUM_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Sodium REQUIRED_VARS SODIUM_LIBRARY SODIUM_INCLUDE_DIR)

MARK_AS_ADVANCED(SODIUM_LIBRARY SODIUM_INCLUDE_DIR)


