SUMMARY = "quectel bt lib"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://quectel-core/ql-bluetooth"

SRC_DIR = "${WORKSPACE}/quectel-core/ql-bluetooth"
S = "${WORKDIR}/quectel-core/ql-bluetooth"

DEPENDS = "btvendorhal libcutils ql-mcm-api"


FILES_${PN} += "${libdir}/"

FILES_SOLIBSDEV = ""


do_install() {
    install -d ${D}${libdir}
    install -m 0755 lib/* ${D}${libdir}
}

