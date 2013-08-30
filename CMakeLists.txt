cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${CMAKE_SOURCE_DIR}/vendor/sserialize/cmake/)
include_directories(
	${CMAKE_SOURCE_DIR}
	${CMAKE_CURRENT_BINARY_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/vendor/sserialize/include
	${CMAKE_CURRENT_SOURCE_DIR}/vendor/osmpbf
)

MACRO(ADD_TARGET_PROPERTIES _target _name)
	SET(_properties)
	FOREACH(_prop ${ARGN})
		SET(_properties "${_properties} ${_prop}")
	ENDFOREACH(_prop)
	GET_TARGET_PROPERTY(_old_properties ${_target} ${_name})
	MESSAGE(STATUS "adding property to ${_target} ${_name}:" ${_properties})
	IF(NOT _old_properties)
		# in case it's NOTFOUND
		SET(_old_properties)
	ENDIF(NOT _old_properties)
	SET_TARGET_PROPERTIES(${_target} PROPERTIES ${_name} "${_old_properties} ${_properties}")
ENDMACRO(ADD_TARGET_PROPERTIES)


PROJECT(osmgraph-tools)
SET(PACKAGE_NAME ${CMAKE_PROJECT_NAME})
SET(PACKAGE_VERSION 0.1)

# set(DEBUG_FLAGS "${DEBUG_FLAGS} -Wall")#-Wconversion -Wextra
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -Wall -Wno-unknown-pragmas")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++0x -Wall -Wno-unknown-pragmas")

add_subdirectory(vendor/osmpbf osmpbf)
#add_subdirectory(vendor/sserialize sserialize)
add_subdirectory(creator creator)

add_subdirectory(graphs)