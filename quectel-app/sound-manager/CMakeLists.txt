project(sound-manager LANGUAGES C)
cmake_minimum_required(VERSION 2.8.11)

# package version
find_package (Threads)

find_library(ALSA_INTF_LIBS NAMES alsa_intf_msm8909 PATH_SUFFIXES lib)
find_library(QLMISC_LIBS NAMES ql_misc PATH_SUFFIXES lib)
find_library(QLRIL_LIBS NAMES ql_ril PATH_SUFFIXES lib)
find_library(LOG_LIBS NAMES log PATH_SUFFIXES lib)
find_library(CUTILS_LIBS NAMES cutils PATH_SUFFIXES lib)
find_library(FILEPARSER_LIBS NAMES fileparser PATH_SUFFIXES lib)
find_library(QAUDIO_LIBS NAMES qaudio PATH_SUFFIXES lib)

#add_definitions (-Wno-format-security)
#set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -std=c++11")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

add_subdirectory(src)
