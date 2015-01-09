cmake_minimum_required(VERSION 2.8)
project(readers)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})
find_package(LIBRT REQUIRED)

set(DEFAULT_INCLUDE_DIRS
	${PROTOBUF_INCLUDE_DIRS}
	${ZLIB_INCLUDE_DIRS}
	${PROJECT_GLOBAL_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x -Wall")

include_directories(${DEFAULT_INCLUDE_DIRS})

set(LINK_LIBS

)

set(LIB_SOURCES_CPP
	fmibinaryreader.cpp
)

add_library("${PROJECT_NAME}" ${LIB_SOURCES_CPP})
target_link_libraries("${PROJECT_NAME}" ${LINK_LIBS})

add_executable(fmibinaryreader_example fmibinaryreader_example.cpp ${LIB_SOURCES_CPP})
target_link_libraries(fmibinaryreader_example ${LINK_LIBS})

