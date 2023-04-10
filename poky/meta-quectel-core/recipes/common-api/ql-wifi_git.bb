SUMMARY = "quectel wifi lib"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://quectel-core/ql-wifi"

SRC_DIR = "${WORKSPACE}/quectel-core/ql-wifi"
S = "${WORKDIR}/quectel-core/ql-wifi"

DEPENDS = "wpa-supplicant-qcacld"

PACKAGES = "${PN}"
FILES_${PN} += "${libdir}/*" 
#FILES_${PN}-dbg += "${libdir}/.debug/*"
FILES_${PN} += "${includedir}/*"

SOLIBS = ".so"
FILES_SOLIBSDEV = ""
INSANE_SKIP_${PN} = "dev-so already-stripped"

BBCLASSEXTEND = "native nativesdk"

do_install() {
    install -d ${D}${libdir}
    install -m 0755 lib/libql_wifi.so ${D}${libdir}
    install -d ${D}${libdir}/pkgconfig
    install -m 0755 lib/pkgconfig/wifi.pc ${D}${libdir}/pkgconfig

    install -d ${D}${includedir}/wifi
    install -m 0644 ${WORKSPACE}/quectel-core/ql-wifi/inc/ql_wifi.h ${D}${includedir}/wifi
}

do_package_qa() {
}

PACKAGE_STRIP = "no"
