set (SRC_FILES
    c_printf.c
	qlril_api_test.c
)

include_directories (
    ${PROJECT_SOURCE_DIR}
)

#add_definitions (-Wno-format-security)

add_executable (qlril-api-test ${SRC_FILES})

target_include_directories(qlril-api-test INTERFACE
    $<INSTALL_INTERFACE:include/>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/>
)

target_link_libraries(qlril-api-test
    ql_ril
    ${CMAKE_THREAD_LIBS_INIT}
)

install(TARGETS qlril-api-test EXPORT ${EXPORT_TARGETS_NAME}TARGETS
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION Frameworks
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    COMPONENT core
)
