#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

SUMMARY = "Simple supl ni app"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://quectel-app/ql-supl"

S = "${WORKDIR}/quectel-app/ql-supl"

inherit cmake distutils3-base pkgconfig

DEPENDS = "ql-mcm-api"
DEPENDS += "loc-api-v02"
DEPENDS += "loc-hal"
DEPENDS += "qmi"
DEPENDS += "qmi-framework"
DEPENDS += "cmake (>= 2.6.3)"

BBCLASSEXTEND = "native nativesdk"
#EXTRA_OECMAKE +="-DBUILD_SHARED_LIBS=OFF"
EXTRA_OECMAKE_append += " -DCMAKE_SKIP_RPATH=ON"
EXTRA_OECMAKE_append += " -DCMAKE_INSTALL_DO_STRIP=1"

do_install() {
     install -d ${D}${bindir}
     install -m 0755 ${WORKDIR}/build/src/ql-supl ${D}${bindir}

     install -d ${D}${sysconfdir}/
     install -m 0644 ${S}/supl_test.conf ${D}${sysconfdir}/supl_test.conf

     install -d ${D}${systemd_unitdir}/system/
     install -m 0644 ${S}/ql-supl.service.in ${D}${systemd_unitdir}/system/ql-supl.service

#install -d ${D}${systemd_unitdir}/system/multi-user.target.wants
#ln -sf ${systemd_unitdir}/system/ql-supl.service \
#${D}${systemd_unitdir}/system/multi-user.target.wants/ql-supl.service
}

FILES_${PN} += "${bindir}/" 
FILES_${PN} += "${systemd_unitdir}/system/"
