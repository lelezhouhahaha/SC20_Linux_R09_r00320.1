add_executable (ql-gnss-test ql-gnss-test.c )

include_directories (
	./
)

target_include_directories(ql-gnss-test INTERFACE
    $<INSTALL_INTERFACE:include/>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include/>
    PRIVATE ${QMI_INCLUDE_DIRS}
)

target_link_libraries (ql-gnss-test
	mcm
	mcmipc
    ${QMI_LIBS}
    ${QMI_COMMON_SO_LIBS}
    ${QMI_ENCDEC_LIBS}
    ${QMI_CCI_LIBS}
    ${QMI_CSI_LIBS}
    ${QMI_SAP_LIBS}
    ${QMI_SERVICE_LIBS}
    ${CUTILS_LIBS}
)
