cmake_minimum_required(VERSION 3.0)
project(convert)

set(CWS6_CPP_VERSION "c++14")
if(MSVC)
	set(CWS6_CPP_VERSION "/std:${CWS6_CPP_VERSION}")
else()
	set(CWS6_CPP_VERSION "-std=${CWS6_CPP_VERSION}")
endif()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CWS6_CPP_VERSION}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${CWS6_CPP_VERSION}")

include_directories(../)

set(JSON_INCLUDE_DIR $ENV{JSON_INCLUDE_DIR})
include_directories(${JSON_INCLUDE_DIR})

add_subdirectory(../ lookup)

file(GLOB srcs *.cpp *.h *.hpp)
add_executable(${PROJECT_NAME} ${srcs})
target_link_libraries(${PROJECT_NAME} lookup)