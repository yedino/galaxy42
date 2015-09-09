cmake_minimum_required(VERSION 2.8.3)
project (antinet)

set(CMAKE_CXX_FLAGS "-std=c++11 -g3 -O0 -Wall -pedantic")
#-Wno-error=undefined-variable ")

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR})

file(GLOB SOURCE_FILES "*.cpp" # All files in directory:
)

source_group(SRC_FILES FILES ${SOURCE_FILES})

add_executable(antinet.elf
	${SOURCE_FILES})

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
  
