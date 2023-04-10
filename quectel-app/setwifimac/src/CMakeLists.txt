# ************ Client program ************

add_executable ( setwifimac setwifimac.c )

#include_directories (
#    ${CMAKE_SOURCE_DIR}/include
#)

target_link_libraries (setwifimac
	${QLMISC_LIBS}
	${CUTILS_LIBS}
)
