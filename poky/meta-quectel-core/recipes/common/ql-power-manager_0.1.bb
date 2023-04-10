#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

SUMMARY = "Simple power manager application"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://quectel-core/ql-power-manager"
SRC_URI += "file://quectel-core/ql-power-manager/ql-power-manager.service.in"
SRC_URI += "file://quectel-core/ql-power-manager/ql_power_manager.conf"

S = "${WORKDIR}/quectel-core/ql-power-manager"
SRC_DIR = "${WORKSPACE}/quectel-core/ql-power-manager"

do_compile() {
}


do_install() {
     install -d ${D}${bindir}
     install -m 0755 ${S}/usr/bin/ql-power-manager ${D}${bindir}

     install -d ${D}${sysconfdir}/
     install -m 0644 ${S}/ql_power_manager.conf ${D}${sysconfdir}/ql_power_manager.conf

     install -d ${D}${systemd_unitdir}/system/
     install -m 0644 ${S}/ql-power-manager.service.in ${D}${systemd_unitdir}/system/ql-power-manager.service

     install -d ${D}${sysconfdir}/systemd/system/multi-user.target.wants

     ln -sf ${systemd_unitdir}/system/ql-power-manager.service \
        ${D}${sysconfdir}/systemd/system/multi-user.target.wants/ql-power-manager.service
}

do_package_qa() {
}

FILES_${PN} += "/lib/systemd/*"
FILES_${PN} += "${bindir}/"
FILES_${PN} += "${sysconfdir}"

#FILES_SOLIBSDEV = ""
