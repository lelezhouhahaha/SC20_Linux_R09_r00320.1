project(ql-audio-test C)
cmake_minimum_required(VERSION 2.6.3)
#set ( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_admin )

# package version
find_package (PkgConfig REQUIRED)

find_library(QLAUDIO_LIBS NAMES ql_audio_api PATH_SUFFIXES lib)

add_subdirectory(src)
