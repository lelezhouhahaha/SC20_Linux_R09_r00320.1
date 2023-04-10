project(SETWIFIMAC C)
cmake_minimum_required(VERSION 2.6.3)
#set ( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_admin )

# package version
find_package (PkgConfig REQUIRED)

find_library(QLMISC_LIBS NAMES ql_misc PATH_SUFFIXES lib)
find_library(CUTILS_LIBS NAMES cutils PATH_SUFFIXES lib)

add_subdirectory(src)

enable_testing()

add_test(test_run
    src/setwifimac
)
