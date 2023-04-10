#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

SUMMARY = "Simple mobap-switch demo"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://quectel-app/mobap-switch/"

S = "${WORKDIR}/quectel-app/mobap-switch"
SRC_DIR = "${WORKSPACE}/quectel-app/mobap-switch"

inherit cmake

DEPENDS = "cmake (>= 2.6.3)"
DEPENDS = "mcm-core ql-mcm-api"

#EXTRA_OECMAKE +="-DBUILD_SHARED_LIBS=OFF"
EXTRA_OECMAKE_append += " -DCMAKE_SKIP_RPATH=ON"
EXTRA_OECMAKE_append += " -DCMAKE_INSTALL_DO_STRIP=1"

FILES_${PN} += "/lib/systemd/*"
FILES_${PN} += "/etc/systemd/*"

do_install() {
    install -d ${D}${bindir}
    install -m 0755 ${WORKDIR}/build/mobap-switch -D ${D}${bindir}/

    install -d ${D}${systemd_unitdir}/system/
    install -m 0644 ${SRC_DIR}/mobap-switch.service ${D}${systemd_unitdir}/system/mobap-switch.service


    install -d ${D}${sysconfdir}/systemd/system/multi-user.target.wants/

    ln -sf ${systemd_unitdir}/system/mobap-switch.service \
    ${D}${sysconfdir}/systemd/system/multi-user.target.wants/mobap-switch.service


    install -d ${D}${sysconfdir}/
    install -m 0644 ${S}/mobap_switch.conf ${D}${sysconfdir}/mobap_switch.conf

}

FILES_${PN} += "${bindir}/" 

FILES_SOLIBSDEV = ""
