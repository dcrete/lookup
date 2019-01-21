cmake_minimum_required(VERSION 3.0)
project(lookup)

include_directories(./)
set(JSON_INCLUDE_DIR $ENV{JSON_INCLUDE_DIR})
include_directories(${JSON_INCLUDE_DIR})

file(GLOB srcs lookup/*.cpp lookup/*.h lookup/*.hpp)
add_library(${PROJECT_NAME} ${srcs})