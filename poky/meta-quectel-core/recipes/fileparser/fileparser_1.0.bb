#
# Yocto Project Development Manual.
#
SUMMARY = "fileparser library"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESPATH =+ "${WORKSPACE}/quectel-core/:"
SRC_URI = "file://fileparser/"

SRC_DIR = "${WORKSPACE}/quectel-core/fileparser"
S = "${WORKDIR}/fileparser"

inherit cmake

#EXTRA_OECMAKE +="-DBUILD_SHARED_LIBS=OFF"
EXTRA_OECMAKE_append += " -DCMAKE_INSTALL_DO_STRIP=1"
EXTRA_OECMAKE_append += " -DCMAKE_SKIP_RPATH=ON"

PACKAGES = "${PN}"
FILES_${PN} += "${libdir}/" 
FILES_${PN}-dbg += "${libdir}/.debug/*"
FILES_${PN} += "${includedir}/*"
FILES_SOLIBSDEV = ""
INSANE_SKIP_${PN} = "dev-so"

do_package_qa() {
}
