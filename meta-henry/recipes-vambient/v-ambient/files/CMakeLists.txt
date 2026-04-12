cmake_minimum_required(VERSION 3.10)
project(v-ambient)

set(CMAKE_CXX_STANDARD 23)

find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBS REQUIRED sdbusplus libsystemd)

#add_executable(v-ambient v-ambient.cpp)
add_executable(v-ambient-assoc v-ambient-assoc.cpp)

#target_link_libraries(v-ambient ${LIBS_LIBRARIES} boost_system)
target_link_libraries(v-ambient-assoc ${LIBS_LIBRARIES} boost_system)

install(TARGETS v-ambient-assoc DESTINATION ${CMAKE_INSTALL_BINDIR})
