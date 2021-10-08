cmake_minimum_required(VERSION 3.16)
project(osmgraph-tools)

set(CMAKE_MODULE_PATH
	${CMAKE_SOURCE_DIR}/cmake
	${CMAKE_SOURCE_DIR}/vendor/sserialize/cmake/
)


add_subdirectory(vendor/sserialize sserialize)
add_subdirectory(vendor/osmpbf osmpbf)

add_subdirectory(creator creator)
add_subdirectory(graphs graphs)
add_subdirectory(readers readers)
