# ************ Client program ************

add_executable ( ql-audio-test ql_audio_test.c )

#include_directories (
#    ${CMAKE_SOURCE_DIR}/include
#)

target_link_libraries (ql-audio-test
	${QLAUDIO_LIBS}
)

install(TARGETS ql-audio-test DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)
