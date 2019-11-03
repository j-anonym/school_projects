cmake_minimum_required(VERSION 3.15)
project(proj)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

SET(GCC_COVERAGE_LINK_FLAGS    "-lpcap")

SET(CMAKE_MODULE_PATH "/home/janonym/.git/school_projects/sem_5/isa/proj")
find_package(PCAP REQUIRED)

set(SOURCE_FILES d6r.cpp d6r.h)
add_executable(d6r ${SOURCE_FILES})
target_link_libraries(d6r ${PCAP_LIBRARY})