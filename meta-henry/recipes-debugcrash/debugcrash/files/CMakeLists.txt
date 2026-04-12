cmake_minimum_required(VERSION 3.10)
project(debugcrash)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")

add_executable(debugcrash debugcrash.cpp)


install(TARGETS debugcrash DESTINATION bin)
