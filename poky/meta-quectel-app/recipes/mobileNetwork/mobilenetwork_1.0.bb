#
# 2020/9/23 peeta.chen
# recipe for mobile network
# Yocto Project Development Manual.
#

SUMMARY = "Mobile Network Application"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://quectel-app/qt/mobileNetwork"

S = "${WORKDIR}/quectel-app/qt/mobileNetwork"
SRC_DIR = "${WORKSPACE}/quectel-app/qt/mobileNetwork"

require recipes-qt/qt5/qt5.inc
DEPENDS = "qtbase qtbase-native qtdeclarative qtquickcontrols2"
DEPENDS += "ql-misc"
DEPENDS += "qlrild"

FILES_${PN}-qmlplugins += " \
    ${OE_QMAKE_PATH_QML}/QtQuick/Controls/Shaders \
    ${OE_QMAKE_PATH_QML}/QtQuick/Dialogs/qml/icons.ttf \
"

inherit qmake5

do_install() {
}

do_package_qa() {
}

FILES_${PN} += "${bindir}/*"
