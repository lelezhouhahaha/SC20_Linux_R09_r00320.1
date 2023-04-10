aux_source_directory(. DIR_LIB_SRCS)

add_executable ( network-manager network_manager.c )

target_link_libraries ( network-manager 
    ${CUTILS_LIBS}
	${QLMISC_LIBS}
	${QLRIL_LIBS}
	${FILEPARSER_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
    ${LOG_LIBS}
    ${UTILS_LIBS}
)
