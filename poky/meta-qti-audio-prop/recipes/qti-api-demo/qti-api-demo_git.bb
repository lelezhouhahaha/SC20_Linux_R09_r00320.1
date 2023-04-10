inherit autotools qcommon qlicense qprebuilt

DESCRIPTION = "qti api demo"
PR = "r0"

SRC_DIR = "${WORKSPACE}/audio/mm-audio/qti-api-demo/"
S = "${WORKDIR}/audio/mm-audio/qti-api-demo/"

DEPENDS = "glib-2.0 tinyalsa libcutils libhardware soundtrigger qti-api audiohal qahw"
DEPENDS_append_msm8909 = " qti-audio-server binder"
DEPENDS_append_apq8017 = " qti-audio-server binder"
EXTRA_OECONF += "--with-glib --program-suffix=${HALBINSUFFIX}"
EXTRA_OECONF += "BOARD_SUPPORTS_QSTHW_API=true"
EXTRA_OECONF_append_apq8017 = " BOARD_SUPPORTS_QTI_AUDIO_SERVER=true"
EXTRA_OECONF_append_msm8909 = " BOARD_SUPPORTS_QTI_AUDIO_SERVER=true"
FILES_${PN} += "${libdir}/*"
FILES_${PN} += "${includedir}/*"
FILES_SOLIBSDEV = ""
