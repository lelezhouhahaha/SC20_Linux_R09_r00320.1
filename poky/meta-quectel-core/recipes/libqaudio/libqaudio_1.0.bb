# qaudio library

SUMMARY = "qaudio library"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

PROVIDES = "qaudio"

DEPENDS += "liblog"
DEPENDS += "libcutils"
DEPENDS += "libutils"
DEPENDS += "ql-misc"
DEPENDS += "fileparser"
DEPENDS += "system-media"
DEPENDS += "audiohal"

FILESPATH =+ "${WORKSPACE}/quectel-core/:"
SRC_URI = "file://libqaudio/"

SRC_DIR = "${WORKSPACE}/quectel-core/libqaudio"
S = "${WORKDIR}/libqaudio"

do_install() {
	install -d ${D}${includedir}
	install -m 0444 ${S}/include/qaudio.h -D ${D}${includedir}

	install -d ${D}${libdir}
	install -m 0644 ${S}/lib/*.so -D ${D}${libdir}/
}

FILES_${PN} = "${libdir}/" 
FILES_${PN} += "${includedir}/*"

FILES_SOLIBSDEV = ""
PACKAGE_STRIP = "no"
