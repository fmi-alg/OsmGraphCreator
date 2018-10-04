cmake_minimum_required(VERSION 2.8)
project(readers)

set(LIB_SOURCES_CPP
	fmibinaryreader.cpp
	fmitextreader.cpp
)

add_library(${PROJECT_NAME} STATIC ${LIB_SOURCES_CPP})

add_executable(fmibinaryreader_example fmibinaryreader_example.cpp ${LIB_SOURCES_CPP})
add_executable(fmitextreader_example fmitextreader_example.cpp ${LIB_SOURCES_CPP})

