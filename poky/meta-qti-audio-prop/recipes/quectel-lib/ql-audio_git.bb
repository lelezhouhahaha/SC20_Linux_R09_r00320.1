#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

inherit autotools qcommon qlicense qprebuilt


DESCRIPTION = "quectel audio api"
PR = "r0"

FILESPATH =+ "${WORKSPACE}:"
#SRC_URI = "file://hardware/qcom/audio/quectel-lib"

S = "${WORKDIR}/hardware/qcom/audio/quectel-lib/"
SRC_DIR = "${WORKSPACE}/hardware/qcom/audio/quectel-lib/"


SOLIBS = ".so"
FILES_SOLIBSDEV = ""

do_package_qa() {
}

do_install() {
	install -d ${D}${libdir}
	install -m 0755 ${SRC_DIR}*.so ${D}${libdir}

	install -d ${D}${includedir}
	install -m 0644 ${SRC_DIR}*.h ${D}${includedir}
}

deltask do_compile
