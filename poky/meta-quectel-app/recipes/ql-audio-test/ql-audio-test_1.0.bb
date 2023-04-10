#
# Yocto Project Development Manual.
#
SUMMARY = "Test ql_audio_api library API utils"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESPATH =+ "${WORKSPACE}/quectel-app/:"
SRC_URI = "file://ql-audio-test/"

SRC_DIR = "${WORKSPACE}/quectel-app/ql-audio-test"
S = "${WORKDIR}/ql-audio-test"

INSANE_SKIP_${PN} += "dev-deps"
INSANE_SKIP_${PN} = "file-rdeps"

inherit cmake distutils-base pkgconfig

DEPENDS += "audiohal ql-audio"

#EXTRA_OECMAKE +="-DBUILD_SHARED_LIBS=OFF"
EXTRA_OECMAKE_append += " -DCMAKE_SKIP_RPATH=ON"
EXTRA_OECMAKE_append += " -DCMAKE_INSTALL_DO_STRIP=1"

FILES_${PN} = "${bindir}/" 

FILES_SOLIBSDEV = ""
