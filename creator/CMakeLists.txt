cmake_minimum_required(VERSION 3.16)
project(creator)

find_package(Protobuf REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Ragel REQUIRED)
find_package(LIBRT REQUIRED)

option(CONFIG_CREATOR_SUPPORT_STRING_CARRY_OVER OFF)
option(CONFIG_CREATOR_SUPPORT_SSERIALIZE_OFFSET_ARRAY_TARGET OFF)

set(RAGEL_FLAGS "-G2")

RAGEL_PARSER(MaxSpeedParser.rl)

set(LINK_LIBS
	sserialize
	osmpbf
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

add_executable(${PROJECT_NAME} ${SOURCES_CPP})
target_link_libraries(${PROJECT_NAME} ${LINK_LIBS})
target_include_directories(${PROJECT_NAME} PRIVATE ${ZLIB_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})
if (CONFIG_CREATOR_SUPPORT_STRING_CARRY_OVER)
	target_compile_definitions(${PROJECT_NAME} PRIVATE CONFIG_SUPPORT_STRING_CARRY_OVER)
endif()
if (CONFIG_CREATOR_SUPPORT_SSERIALIZE_OFFSET_ARRAY_TARGET)
	target_compile_definitions(${PROJECT_NAME} PRIVATE CONFIG_SUPPORT_SSERIALIZE_OFFSET_ARRAY_TARGET)
endif()
