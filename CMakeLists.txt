cmake_minimum_required(VERSION 2.8)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMakeFiles/modules/")

PROJECT(iconv++)

INCLUDE (CheckIncludeFiles)
include (FindPkgConfig)

# build type
set(CMAKE_BUILD_TYPE Debug)

# compile option (Debug) 
set(CMAKE_CXX_FLAGS_DEBUG "-g -std=c++0x")
set(CMAKE_C_FLAGS_DEBUG "-g")

# compile option (Release)
set(CMAKE_CXX_FLAGS_RELEASE "-W -Wall -O3 -std=c++0x")
set(CMAKE_C_FLAGS_RELEASE "-W -Wall -O3")

#if you don't want the full compiler output, remove the following line
set(CMAKE_VERBOSE_MAKEFILE OFF)

set(Boost_USE_MULTITHREADED ON)
find_package(Boost 1.42.0 COMPONENTS system REQUIRED )
if (NOT Boost_FOUND)
  message(SEND_ERROR "Boost not found")
endif (NOT Boost_FOUND)

INCLUDE_DIRECTORIES( ${CMAKE_CURRENT_SOURCE_DIR}/include ${CMAKE_CURRENT_BINARY_DIR}/include ${Boost_INCLUDE_DIRS} )
subdirs( include src )
