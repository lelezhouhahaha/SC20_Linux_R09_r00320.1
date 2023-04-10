AM_CFLAGS = -Wall \
        -Wundef \
        -Wstrict-prototypes \
        -fno-inline \
        -fno-short-enums \
        -fpic -g

ACLOCAL_AMFLAGS = -I m4
AM_CPPFLAGS = -D__packed__= \
        -I$(WORKSPACE)/quectel-core/mm-audio/usr/include/alsa-intf-msm8909 \
        -DFEATURE_LINUX \
        -DFEATURE_NATIVELINUX \
	-DMULTI_PROCESS_CONFIG \
        $(QMIFRAMEWORK_CFLAGS) \
        $(LIBALSAINTF_CFLAGS) \
        $(QMI_CFLAGS) \
        $(DIAG_FLAGS) 

c_sources = qlplay.c 
requiredlibs = -lgthread-2.0 -lpthread -lglib-2.0 -lalsa_intf_msm8909
bin_PROGRAMS = qlplay qlmix qlrec
#init_ddir = ${sysconfdir}/init.d
qlplay_CC = @CC@
qlplay_SOURCES = qlplay.c
qlplay_LDADD = -lpthread -lm -lalsa_intf_msm8909 $(QMIFRAMEWORK_LIBS) -ldl $(DIAG_LIBS) 
qlplay_LDFLAGS = -rdynamic
qlplay_CFLAGS = -Dstrlcat=g_strlcat -Dstrlcpy=g_strlcpy 
qlplay_CPPFLAGS = -Dstrlcat=g_strlcat -Dstrlcpy=g_strlcpy $(AM_CPPFLAGS) 
qlplay_LDADD = ${requiredlibs}
pkgconfigdir = $(libdir)/pkgconfig

EXTRA_DIST = $(pkgconfig_DATA)
c_sources = qlrec.c 

#init_ddir = ${sysconfdir}/init.d
qlrec_CC = @CC@
qlrec_SOURCES = qlrec.c
qlrec_LDADD = -lpthread -lm -lalsa_intf_msm8909 $(QMIFRAMEWORK_LIBS) -ldl $(DIAG_LIBS) 
qlrec_LDFLAGS = -rdynamic
qlrec_CFLAGS = -DUSE_GLIB -Dstrlcat=g_strlcat -Dstrlcpy=g_strlcpy 
qlrec_CPPFLAGS = -DUSE_GLIB -Dstrlcat=g_strlcat -Dstrlcpy=g_strlcpy $(AM_CPPFLAGS)

c_sources = qlmix.c 
#bin_PROGRAMS = qlmix
#init_ddir = ${sysconfdir}/init.d
qlmix_CC = @CC@
qlmix_SOURCES = qlmix.c
qlmix_LDADD = -lpthread -lm -lalsa_intf_msm8909 $(QMIFRAMEWORK_LIBS) -ldl $(DIAG_LIBS)
qlmix_LDFLAGS = -rdynamic
qlmix_CFLAGS = -DUSE_GLIB -Dstrlcat=g_strlcat -Dstrlcpy=g_strlcpy
qlmix_CPPFLAGS = -DUSE_GLIB -Dstrlcat=g_strlcat -Dstrlcpy=g_strlcpy $(AM_CPPFLAGS)
