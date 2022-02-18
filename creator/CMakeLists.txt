cmake_minimum_required(VERSION 3.16)
project(creator)

find_package(Protobuf REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Ragel REQUIRED)

set(RAGEL_FLAGS "-G2")

RAGEL_PARSER(MaxSpeedParser.rl)

set(LINK_LIBS
	sserialize
	osmpbf
	protobuf::libprotobuf
	ZLIB::ZLIB
)

set(SOURCES_CPP
	main.cpp
	GraphWriter.cpp
	WeightCalculator.cpp
	MaxSpeedParser.cpp
	RamGraph.cpp
)

set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
add_executable(${PROJECT_NAME} ${SOURCES_CPP})
target_link_libraries(${PROJECT_NAME} ${LINK_LIBS})
target_include_directories(${PROJECT_NAME} PRIVATE ${ZLIB_INCLUDE_DIRS} ${CMAKE_CURRENT_SOURCE_DIR})

option(CONFIG_CREATOR_COPY_TAGS "Add tags to nodes and edges. Format is defined at http://overpass-api.de/output_formats.html#json")
if (CONFIG_CREATOR_COPY_TAGS)
	target_compile_definitions(${PROJECT_NAME} PRIVATE CONFIG_CREATOR_COPY_TAGS)
endif()
option(CONFIG_CREATOR_SUPPORT_SSERIALIZE_OFFSET_ARRAY_TARGET "Support creating static graph using " OFF)
if (CONFIG_CREATOR_SUPPORT_SSERIALIZE_OFFSET_ARRAY_TARGET)
	target_compile_definitions(${PROJECT_NAME} PRIVATE CONFIG_SUPPORT_SSERIALIZE_OFFSET_ARRAY_TARGET)
endif()
