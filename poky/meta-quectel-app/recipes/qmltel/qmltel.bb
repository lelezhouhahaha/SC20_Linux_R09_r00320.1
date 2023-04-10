#
# 2019/8/29	javed.wu 
# recipe for quec_launcher
# Yocto Project Development Manual.
#

SUMMARY = "quec_launcher application"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://quectel-app/qt/qmltel"

S = "${WORKDIR}/quectel-app/qt/qmltel"
SRC_DIR = "file://quectel-app/qt/qmltel"

require recipes-qt/qt5/qt5.inc

DEPENDS += "qtbase qtbase-native qtdeclarative qtquickcontrols qtquickcontrols2"
#DEPENDS += "qtdeclarative"
DEPENDS += "ql-mcm-api"

#FILES_${PN}      = "${libdir}/*.so ${libdir}/*.so.* ${libdir}/*.so.*.*.* ${sysconfdir}/* ${bindir}/* ${libdir}/pkgconfig/* ${base_prefix}/*"
#FILES_${PN}-dev  += "${libdir}/*.la ${includedir}"
#FILES_${PN} += "/usr/bin/*"

inherit qmake5


do_install() {
	install -d ${D}${bindir}
	install -m 0755 qmltel ${D}${bindir}
}
