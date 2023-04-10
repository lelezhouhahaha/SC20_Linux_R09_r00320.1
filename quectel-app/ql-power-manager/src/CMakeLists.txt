# ************ Client program ************

add_executable ( ql-power-manager ql_power_manager.c )

#include_directories (
#    ${CMAKE_SOURCE_DIR}/include
#)

target_link_libraries ( ql-power-manager
    ${CMAKE_THREAD_LIBS_INIT}
    ${QL_MCM_API_LIBRARIES}
    ${QL_WIFI_LIBRARIES}
    ${CUTILS_LIBS}
    ${QLRIL_LIBS}
    ${QL_MISC}
)

install(TARGETS ql-power-manager DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
