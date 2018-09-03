cmake_minimum_required(VERSION 3.1.0)
project(simpleGui)

set (CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

set(WITH_TOOLS NO CACHE BOOL "")
add_subdirectory(../depends/libqrencode build)

file(GLOB src
    *.cpp
    *.h
    *.ui
    *.qrc
)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt5 5.6 COMPONENTS Core Gui Network Multimedia Widgets REQUIRED)

find_package(Sodium REQUIRED)

add_executable(simpleGui ${src})
target_link_libraries(simpleGui
    Qt5::Core
    Qt5::Gui
    Qt5::Network
    Qt5::Multimedia
    Qt5::Widgets
    ${SODIUM_LIBRARY}
    qrencode
)
target_include_directories(simpleGui PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}
    ../depends/json/src
    ${SODIUM_INCLUDE_DIR}
    ../depends/libqrencode
)
