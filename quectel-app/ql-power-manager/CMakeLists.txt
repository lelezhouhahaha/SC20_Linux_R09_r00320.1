project(ql-power-manager C)
cmake_minimum_required(VERSION 2.6.3)
#set ( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_admin )

#check_include_file (pthread.h HAVE_PTHREAD_H)

# package version
find_package (PkgConfig REQUIRED)

find_package (Threads)
set ( HAVE_LIBPTHREAD ${Threads_FOUND} )

find_library(QL_MCM_API_LIBRARIES NAMES ql_mcm_api PATH_SUFFIXES lib)
find_library(QL_WIFI_LIBRARIES NAMES ql_wifi PATH_SUFFIXES lib)
find_library(CUTILS_LIBS NAMES cutils PATH_SUFFIXES lib)
find_library(QLRIL_LIBS NAMES ql_ril PATH_SUFFIXES lib)
find_library(QL_MISC NAMES ql_misc PATH_SUFFIXES lib)

add_subdirectory(src)
