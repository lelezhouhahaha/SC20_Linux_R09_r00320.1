#
# 2019/7/15 peeta.chen
# recipe for settings
# Yocto Project Development Manual.
#

SUMMARY = "Settings application"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://quectel-app/qt/settings"

S = "${WORKDIR}/quectel-app/qt/settings"
SRC_DIR = "${WORKSPACE}/quectel-app/qt/settings"

require recipes-qt/qt5/qt5.inc
DEPENDS += "qtbase qtbase-native qtdeclarative qtquickcontrols qtquickcontrols2"
DEPENDS += "ql-misc ql-power"

FILES_${PN}-qmlplugins += " \
    ${OE_QMAKE_PATH_QML}/QtQuick/Controls/Shaders \
    ${OE_QMAKE_PATH_QML}/QtQuick/Dialogs/qml/icons.ttf \
"
#FILES_${PN} += "/lib/systemd/*"
#FILES_${PN} += "/etc/systemd/*"

inherit qmake5

do_install() {
		 install -d ${D}${bindir}
	     install -m 0755 settings ${D}${bindir}

         #added by peeta
#         install -d ${D}${systemd_unitdir}/system/
#         install -m 0644 ${WORKDIR}/quectel-app/qt/quec_launcher/quec_launcher.service.in \
#         ${D}${systemd_unitdir}/system/quec_launcher.service
#
#         install -d ${D}${sysconfdir}/systemd/system/local-fs-pre.target.wants/
#
#         ln -sf ${systemd_unitdir}/system/quec_launcher.service \
#         ${D}${sysconfdir}/systemd/system/local-fs-pre.target.wants/quec_launcher.service
}
