#

SUMMARY = "lwm2m qt application"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://quectel-app/qt/lwm2mqtdemo"

S = "${WORKDIR}/quectel-app/qt/lwm2mqtdemo"
SRC_DIR = "${WORKSPACE}/quectel-app/qt/lwm2mqtdemo"

require recipes-qt/qt5/qt5.inc
DEPENDS += "qtbase"

do_install() {
		 install -d ${D}${bindir}
	     install -m 0755 lwm2mqtdemo ${D}${bindir}
}
