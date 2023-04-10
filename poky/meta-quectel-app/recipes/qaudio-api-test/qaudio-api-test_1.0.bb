#
# Yocto Project Development Manual.
#
SUMMARY = "QAUDIO API utils"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESPATH =+ "${WORKSPACE}/quectel-app/:"
SRC_URI = "file://qaudio-api-test/"

SRC_DIR = "${WORKSPACE}/quectel-app/qaudio-api-test"
S = "${WORKDIR}/qaudio-api-test"

INSANE_SKIP_${PN} += "dev-deps"

inherit cmake distutils-base pkgconfig

DEPENDS = "libqaudio"

#EXTRA_OECMAKE +="-DBUILD_SHARED_LIBS=OFF"
EXTRA_OECMAKE_append += " -DCMAKE_SKIP_RPATH=ON"
EXTRA_OECMAKE_append += " -DCMAKE_INSTALL_DO_STRIP=1"

FILES_${PN} = "${bindir}/" 

FILES_SOLIBSDEV = ""
