project(qaudio-api-test C)
cmake_minimum_required(VERSION 2.6.3)
#set ( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_admin )

find_package (Threads)
find_package (PkgConfig REQUIRED)

find_library(QAUDIO_LIBS NAMES qaudio PATH_SUFFIXES lib)
find_library(MATH_LIBS NAMES m PATH_SUFFIXES lib)

add_executable (qaudio-api-test qaudio_api_test.c wav.c)

target_link_libraries (qaudio-api-test
	${CMAKE_THREAD_LIBS_INIT}
	${QAUDIO_LIBS}
	${MATH_LIBS}
)

install(TARGETS qaudio-api-test DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
