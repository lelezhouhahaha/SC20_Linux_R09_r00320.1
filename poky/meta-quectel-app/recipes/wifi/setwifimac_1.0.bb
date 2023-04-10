#
# Yocto Project Development Manual.
#

SUMMARY = "Set WIFI Mac"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://quectel-app/setwifimac/"

S = "${WORKDIR}/quectel-app/setwifimac"
SRC_DIR = "${WORKSPACE}/quectel-app/setwifimac"

inherit cmake

DEPENDS = "ql-misc"
DEPENDS += "cmake (>= 2.6.3)"
#EXTRA_OECMAKE +="-DBUILD_SHARED_LIBS=OFF"
EXTRA_OECMAKE_append += " -DCMAKE_SKIP_RPATH=ON"
EXTRA_OECMAKE_append += " -DCMAKE_INSTALL_DO_STRIP=1"

do_install() {
     install -d ${D}${bindir}
     install -m 0755 ${WORKDIR}/build/src/setwifimac -D ${D}${bindir}/

#install -d ${D}${systemd_unitdir}/system/
#install -m 0644 ${SRC_DIR}/setwifimac.service.in ${D}${systemd_unitdir}/system/setwifimac.service

#install -d ${D}${systemd_unitdir}/system/multi-user.target.wants/
#ln -sf ${systemd_unitdir}/system/setwifimac.service \
#${D}${systemd_unitdir}/system/multi-user.target.wants/setwifimac.service
#chrpath -d ${D}${bindir}/debug-test
}

FILES_${PN} += "${systemd_unitdir}/" 
FILES_${PN} += "${bindir}/" 

FILES_SOLIBSDEV = ""
