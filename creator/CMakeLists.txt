cmake_minimum_required(VERSION 2.8)
project(creator)
include_directories(${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR})
find_package(Protobuf REQUIRED)
find_package(ZLIB REQUIRED)
find_package(GeographicLib REQUIRED)
find_package(Ragel REQUIRED)

set(DEFAULT_INCLUDE_DIRS
	${PROTOBUF_INCLUDE_DIRS}
	${ZLIB_INCLUDE_DIRS}
	${GeographicLib_INCLUDE_DIRS}
	${PROJECT_GLOBAL_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}
	${CMAKE_SOURCE_DIR}/vendor/osmpbf
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x -Wall")

set(RAGEL_FLAGS "-G2")

RAGEL_PARSER(MaxSpeedParser.rl)

include_directories(${DEFAULT_INCLUDE_DIRS})

set(LINK_LIBS
${osmpbf_BINARY_DIR}/libosmpbf.a
${sserialize_BINARY_DIR}/libsserialize.a
${PROTOBUF_LIBRARIES}
${ZLIB_LIBRARIES}
${GeographicLib_LIBRARIES}
)

set(SOURCES_CPP
main.cpp
GraphWriter.cpp
WeightCalculator.cpp
MaxSpeedParser.cpp
RamGraph.cpp
)

add_executable("${PROJECT_NAME}" ${SOURCES_CPP})
target_link_libraries("${PROJECT_NAME}" ${LINK_LIBS})
add_dependencies("${PROJECT_NAME}" osmpbf sserialize)
