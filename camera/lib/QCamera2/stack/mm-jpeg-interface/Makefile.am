ACLOCAL_AMFLAGS = -I m4

curr_path = $(top_srcdir)/QCamera2/stack/mm-jpeg-interface

AM_CFLAGS = -Wall -Wextra -Werror

AM_CPPFLAGS = \
    -I $(WORKSPACE)/frameworks/native/include/media/openmax \
    -I $(curr_path)/inc \
    -I $(curr_path)/../common \
    -I $(curr_path)/../mm-camera-interface/inc \
    -I $(curr_path)/../../.. \
    -I $(curr_path)/../../../mm-image-codec/qexif \
    -I $(curr_path)/../../../mm-image-codec/qomx_core


AM_CPPFLAGS += -D_ANDROID_ -DQCAMERA_REDEFINE_LOG

if TARGET_USES_ION
AM_CPPFLAGS += -DUSE_ION
endif

if TARGET_SUPPORTS_DUAL_JPEG
AM_CPPFLAGS += -DMM_JPEG_CONCURRENT_SESSIONS_COUNT=2
else
AM_CPPFLAGS += -DMM_JPEG_CONCURRENT_SESSIONS_COUNT=1
endif

if JPEG_USE_PIPELINE
AM_CPPFLAGS+= -DMM_JPEG_USE_PIPELINE
endif

AM_CPPFLAGS += -DSYSTEM_HEADER_PREFIX=sys

AM_CPPFLAGS += -include sys/ioctl.h
AM_CPPFLAGS += -include stdint.h

c_sources = \
    src/mm_jpeg_queue.c \
    src/mm_jpeg_exif.c \
    src/mm_jpeg.c \
    src/mm_jpeg_interface.c \
    src/mm_jpeg_ionbuf.c \
    src/mm_jpegdec_interface.c \
    src/mm_jpegdec.c \
    src/mm_jpeg_mpo_composer.c

lib_LTLIBRARIES = libmmjpeg_interface.la
libmmjpeg_interface_la_SOURCES = $(c_sources)
libmmjpeg_interface_la_CFLAGS = $(AM_CFLAGS)
libmmjpeg_interface_la_LIBADD = $(AM_LIBADD) -lcutils -ldl -lpthread -llog
libmmjpeg_interface_la_LIBADD += ../../../mm-image-codec/libqomx_core.la
libmmjpeg_interface_la_LIBADD += ../mm-camera-interface/libmmcamera_interface.la
libmmjpeg_interface_la_LDFLAGS = -shared
libmmjpeg_interface_la_CPPFLAGS = $(AM_CPPFLAGS)
