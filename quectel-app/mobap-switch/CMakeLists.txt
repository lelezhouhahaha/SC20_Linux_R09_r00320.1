project(MOBAP_SWITCH C)
cmake_minimum_required(VERSION 2.6.3)
#set ( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_admin )

# package version
find_package (PkgConfig REQUIRED)

include_directories (
    ${CMAKE_SOURCE_DIR}/include
)
add_executable ( mobap-switch mobap-switch.c )


target_link_libraries (mobap-switch ql_mcm_api)