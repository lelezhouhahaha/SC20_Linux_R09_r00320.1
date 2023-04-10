# ************ Client program ************

add_executable ( ql-supl ql_supl.c )

#include_directories (
#    ${CMAKE_SOURCE_DIR}/include
#)

target_include_directories( ql-supl INTERFACE
    $<INSTALL_INTERFACE:include/>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
    PRIVATE ${QMI_INCLUDE_DIRS}
)

target_link_libraries ( ql-supl
    ${CMAKE_THREAD_LIBS_INIT}
    ${QL_MCM_API_LIBS}
    ${LOC_API_V02_LIBS}
    ${LOC_CORE_LIBS}
    ${GNSS_LIBS}
    ${LOC_ENG_SO_LIBS}
    ${CUTILS_LIBS}
)
