# ************ Client program ************

add_executable ( setbtmac setbtmac.c )

#include_directories (
#    ${QMI_INCLUDE_DIRS}
#)

target_include_directories(setbtmac INTERFACE
    $<INSTALL_INTERFACE:include/>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
    PRIVATE ${QMI_INCLUDE_DIRS}
)

target_link_libraries (setbtmac
    ${QMI_LIBS}
    ${QMI_COMMON_SO_LIBS}
    ${QMI_ENCDEC_LIBS}
    ${QMI_CCI_LIBS}
    ${QMI_CSI_LIBS}
    ${QMI_SAP_LIBS}
    ${QMI_SERVICE_LIBS}
    ${CUTILS_LIBS}
)
