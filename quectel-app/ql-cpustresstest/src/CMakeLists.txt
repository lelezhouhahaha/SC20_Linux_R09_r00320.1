# ************ Client program ************

add_executable ( ql-cpustresstest ql_cpustresstest.c )

#include_directories (
#    ${CMAKE_SOURCE_DIR}/include
#)

#target_include_directories( ql-cpustresstest INTERFACE
#    $<INSTALL_INTERFACE:include/>
#    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
#    PRIVATE ${QMI_INCLUDE_DIRS}
#)

target_link_libraries ( ql-cpustresstest
    ${CMAKE_THREAD_LIBS_INIT}
)
