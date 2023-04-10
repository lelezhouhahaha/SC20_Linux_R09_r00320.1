project(ql-misc-api C)
cmake_minimum_required(VERSION 2.6.3)
#set ( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_admin )

find_package (PkgConfig REQUIRED)

find_library(QLMISC_LIBS NAMES ql_misc PATH_SUFFIXES lib)
find_library(QLRIL_LIBS NAMES ql_ril PATH_SUFFIXES lib)
find_library(MCMAPI_LIBS NAMES ql_mcm_api PATH_SUFFIXES lib)
find_library(CUTILS_LIBS NAMES cutils PATH_SUFFIXES lib)

set (SRC_FILES
    c_printf.c
	ql_misc_test.c
)

include_directories (
    ${PROJECT_SOURCE_DIR}
)

#add_definitions (-Wno-format-security)

add_subdirectory(util)

add_executable (ql-misc-test ${SRC_FILES})

target_include_directories(ql-misc-test INTERFACE
    $<INSTALL_INTERFACE:include/>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/>
)

target_link_libraries(ql-misc-test
	${QLMISC_LIBS}
	${QLRIL_LIBS}
	${MCMAPI_LIBS}
    ${CUTILS_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
)

install(TARGETS ql-misc-test EXPORT ${EXPORT_TARGETS_NAME}TARGETS
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION Frameworks
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    COMPONENT core
)

add_executable (ql-misc-at-test ql_misc_at_test.c)

target_include_directories(ql-misc-at-test INTERFACE
    $<INSTALL_INTERFACE:include/>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/>
)

target_link_libraries(ql-misc-at-test
	${QLMISC_LIBS}
	${QLRIL_LIBS}
	${CUTILS_LIBS}
    ${CMAKE_THREAD_LIBS_INIT}
)

install(TARGETS ql-misc-at-test EXPORT ${EXPORT_TARGETS_NAME}TARGETS
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FRAMEWORK DESTINATION Frameworks
    PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE GROUP_READ GROUP_EXECUTE WORLD_READ WORLD_EXECUTE
    COMPONENT core
)
