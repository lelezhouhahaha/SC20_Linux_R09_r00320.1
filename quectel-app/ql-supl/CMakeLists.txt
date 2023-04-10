project(ql-supl C)
cmake_minimum_required(VERSION 2.6.3)
#set ( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_admin )

#check_include_file (pthread.h HAVE_PTHREAD_H)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# package version
find_package (PkgConfig REQUIRED)
find_package (QMI REQUIRED)

find_package (Threads)
set ( HAVE_LIBPTHREAD ${Threads_FOUND} )
#set ( LIBPOWR_MANAGER_LIBS "m" ${CMAKE_THREAD_LIBS_INIT})

#pkg_check_modules ( QL-MCM-API REQUIRED ql_mcm_api )
#pkg_check_modules ( QL-SYSTEM-INIT REQUIRED ql_system_init.a )

find_library(QL_MCM_API_LIBS NAMES ql_mcm_api PATH_SUFFIXES lib)
find_library(LOC_API_V02_LIBS NAMES loc_api_v02 PATH_SUFFIXES lib)
find_library(LOC_CORE_LIBS NAMES loc_core PATH_SUFFIXES lib)
find_library(GNSS_LIBS NAMES gnss PATH_SUFFIXES lib)
find_library(LOC_ENG_SO_LIBS NAMES loc_eng_so PATH_SUFFIXES lib)
find_library(CUTILS_LIBS NAMES cutils PATH_SUFFIXES lib)

add_subdirectory(src)
#add_subdirectory(tester)

#enable_testing()

#add_test(ql-audio-tester
#    tester/ql-audio-tester
#)
