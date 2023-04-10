#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

SUMMARY = "Simple factory-reset demo"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://quectel-app/factory-reset/"

S = "${WORKDIR}/quectel-app/factory-reset"
SRC_DIR = "${WORKSPACE}/quectel-app/factory-reset"

inherit cmake

DEPENDS = "cmake (>= 2.6.3)"
DEPENDS += "ql-factory-reset"
#EXTRA_OECMAKE +="-DBUILD_SHARED_LIBS=OFF"
EXTRA_OECMAKE_append += " -DCMAKE_SKIP_RPATH=ON"
EXTRA_OECMAKE_append += " -DCMAKE_INSTALL_DO_STRIP=1"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/build/util/factory-reset-test -D ${D}${bindir}/
#chrpath -d ${D}${bindir}/debug-test
}

FILES_${PN} += "${bindir}/" 

FILES_SOLIBSDEV = ""
