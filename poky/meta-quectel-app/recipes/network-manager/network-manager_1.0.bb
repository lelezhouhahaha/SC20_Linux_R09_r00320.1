#
# Yocto Project Development Manual.
#

SUMMARY = "Network manager application"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

inherit update-rc.d
INITSCRIPT_NAME = "network_manager_le"
INITSCRIPT_PARAMS = "start 50 1 2 3 4 5 6 ."

FILESPATH =+ "${WORKSPACE}:"
SRC_DIR = "${WORKSPACE}/quectel-app/network-manager"
SRC_URI = "file://quectel-app/network-manager/"

S = "${WORKDIR}/quectel-app/network-manager"

INSANE_SKIP_${PN} += "dev-deps"

inherit cmake
DEPENDS = "ql-misc"
DEPENDS += "qlrild"
DEPENDS += "fileparser"
DEPENDS += "liblog"
DEPENDS += "libcutils"

do_install() {
	install -d ${D}${bindir}
	install -m 0755 ${WORKDIR}/build/src/network-manager ${D}${bindir}

	install -d ${D}${sysconfdir}/
	install -m 0644 ${S}/network_manager.ini ${D}${sysconfdir}/network_manager.ini

	install -d ${D}${sysconfdir}/init.d
	install -m 0754 ${S}/network_manager_le ${D}${sysconfdir}/init.d/
    
        install -d ${D}${systemd_unitdir}/system/
        install -m 0644 ${S}/lib/systemd/system/network-manager.service ${D}${systemd_unitdir}/system/
 
        install -d ${D}${systemd_unitdir}/system/multi-user.target.wants
        ln -sf ${systemd_unitdir}/system/network-manager.service \
        ${D}${systemd_unitdir}/system/multi-user.target.wants/network-manager.service

}

FILES_${PN} += "${systemd_unitdir}/*"
FILES_${PN} += "${libdir}/"
FILES_${PN} += "${bindir}/"

FILES_SOLIBSDEV = ""
