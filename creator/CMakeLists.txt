cmake_minimum_required(VERSION 2.8)
project(creator)
find_package(Protobuf REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Ragel REQUIRED)
find_package(LIBRT REQUIRED)

include_directories(
	${SSERIALIZE_INCLUDE_DIR}
	${ZLIB_INCLUDE_DIRS}
	${PROJECT_GLOBAL_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++14 -Wall")

set(RAGEL_FLAGS "-G2")

RAGEL_PARSER(MaxSpeedParser.rl)

set(LINK_LIBS
	${osmpbf_BINARY_DIR}/libosmpbf.a
	${SSERIALIZE_LINK_LIBRARIES}
	${PROTOBUF_LIBRARIES}
	${ZLIB_LIBRARIES}
	${LIBRT_LIBRARIES}
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
