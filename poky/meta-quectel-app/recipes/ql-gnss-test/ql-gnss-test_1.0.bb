#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

SUMMARY = "ql-gnss-test"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://quectel-app/ql-gnss-test/"

S = "${WORKDIR}/quectel-app/ql-gnss-test"
SRC_DIR = "${WORKSPACE}/quectel-app/ql-gnss-test"

inherit cmake

DEPENDS = "qmi qmi-framework mcm mcm-core libcutils"
DEPENDS += "cmake (>= 2.6.3)"
#EXTRA_OECMAKE +="-DBUILD_SHARED_LIBS=OFF"
EXTRA_OECMAKE_append += " -DCMAKE_SKIP_RPATH=ON"
EXTRA_OECMAKE_append += " -DCMAKE_INSTALL_DO_STRIP=1"

do_install() {
         install -d ${D}${bindir}
         install -m 0755 ${WORKDIR}/build/src/ql-gnss-test -D ${D}${bindir}/
}

FILES_${PN} += "${systemd_unitdir}/" 
FILES_${PN} += "${bindir}/" 

FILES_SOLIBSDEV = ""
