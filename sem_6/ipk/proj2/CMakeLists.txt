cmake_minimum_required(VERSION 3.15)
project(proj2)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_VERBOSE_MAKEFILE ON)
include(FindPCAP.cmake)



add_executable(proj2 ipk-sniffer.cpp ipk-sniffer.h)

target_link_libraries(proj2 ${PCAP_LIBRARY})