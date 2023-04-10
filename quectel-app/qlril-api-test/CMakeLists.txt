project(qlril-api-test LANGUAGES C)
cmake_minimum_required(VERSION 2.8.11)

# package version
find_package (Threads)

#add_definitions (-Wno-format-security)
#set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Werror")
set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -std=c++11")
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

#add_subdirectory(audio)
add_subdirectory(utils)
