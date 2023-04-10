project(libfileparser C)
cmake_minimum_required(VERSION 2.6.3)
#enable_language(C)

#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99 -g -Wall -pedantic")

#add_definitions(-DDEBUGMSG)

add_definitions(-DNUMBER_OF_MODEMS=1 -DNOSTATS)

add_subdirectory(src)
add_subdirectory(util)
