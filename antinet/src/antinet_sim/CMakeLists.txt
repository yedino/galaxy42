cmake_minimum_required(VERSION 2.8.3)
project (antinet)

set(CMAKE_CXX_FLAGS "-std=c++11 -g3 -O0 -Wall -pedantic")
#-Wno-error=undefined-variable ")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR})

file(GLOB SOURCE_FILES "*.cpp" # All files in directory:
)


file(GLOB HEADERS_FILES "*.hpp" # All files in directory:
)

# remove style files
set(SIM_SOURCES ${SOURCE_FILES})
list(REMOVE_ITEM SIM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/style_tab.cpp)
list(REMOVE_ITEM SIM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/style_tab2.cpp)
list(REMOVE_ITEM SIM_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/style.cpp)

option(USE_OPENGL "Enable/Disable opengl" ON)
if(USE_OPENGL)
       add_definitions(-DUSE_OPENGL=1)
else()
       add_definitions(-DUSE_OPENGL=0)
endif()

option(USE_API_TR "Enable/Disable API_TR" OFF)
if(USE_API_TR)
	add_definitions(-DUSE_API_TR=1)
endif()

add_executable(antinet.elf
	${SIM_SOURCES})

add_custom_target(run
				COMMAND ./antinet.elf
				DEPENDS antinet.elf
				WORKING_DIRECTORY ./)
	
target_link_libraries(antinet.elf
	alleg
	png
	crypto
	cryptopp
	alleggl
	GLU
	GL
	)

 
# agl was here.
  
