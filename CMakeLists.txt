cmake_minimum_required(VERSION 3.16)
project(osmgraph-tools)

set(CMAKE_MODULE_PATH
	${CMAKE_SOURCE_DIR}/cmake
	${CMAKE_SOURCE_DIR}/vendor/sserialize/cmake/
)

set(MY_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unknown-pragmas -Wstrict-overflow=3")
set(MY_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unknown-pragmas -Wstrict-overflow=3")# -Wshorten-64-to-32")

option(OGT_BUILD_WITH_SANITIZERS "Build binaries with additional instrumentation (address, undefined behaviour, thread) sanitizer" OFF)
if (OGT_BUILD_WITH_SANITIZERS)
	set(MY_C_FLAGS "${MY_C_FLAGS} -g -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer")
	set(MY_CXX_FLAGS "${MY_C_FLAGS} -g -fsanitize=address -fsanitize=undefined -fno-omit-frame-pointer")
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MY_C_FLAGS}")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MY_CXX_FLAGS}")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} ${DEBUG_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${DEBUG_FLAGS}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${DEBUG_FLAGS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${DEBUG_FLAGS}")

set(SSERIALIZE_DISABLE_DTS2 ON)
add_subdirectory(vendor/sserialize sserialize)
add_subdirectory(vendor/osmpbf osmpbf)

add_subdirectory(creator creator)
add_subdirectory(graphs graphs)
add_subdirectory(readers readers)
