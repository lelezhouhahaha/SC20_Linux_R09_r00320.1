project(network-manager C)
cmake_minimum_required(VERSION 2.6.3)
set ( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_admin )

find_package (PkgConfig REQUIRED)

find_package (Threads)
set ( HAVE_LIBPTHREAD ${Threads_FOUND} )

set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")

find_library(CUTILS_LIBS NAMES cutils PATH_SUFFIXES lib)
find_library(QLMISC_LIBS NAMES ql_misc PATH_SUFFIXES lib)
find_library(QLRIL_LIBS NAMES ql_ril PATH_SUFFIXES lib)
find_library(FILEPARSER_LIBS NAMES fileparser PATH_SUFFIXES lib)
find_library(LOG_LIBS NAMES log PATH_SUFFIXES lib)

add_subdirectory(src)
