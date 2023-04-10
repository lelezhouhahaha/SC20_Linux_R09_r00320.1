#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

SUMMARY = "Wifi work mode configuration"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://quectel-app/wifi-work-mode"

S = "${WORKDIR}/quectel-app/wifi-work-mode"
SRC_DIR = "${WORKSPACE}/quectel-app/wifi-work-mode"

BBCLASSEXTEND = "native"

do_compile() {
}

do_install() {
     install -d ${D}${sysconfdir}/misc/wifi/
     install -m 0644 ${S}/ifcfg-wlan0 ${D}${sysconfdir}/misc/wifi/
     install -m 0644 ${S}/udhcpd.conf ${D}${sysconfdir}/misc/wifi/
     install -m 0644 ${S}/hostapd-wlan0.conf ${D}${sysconfdir}/misc/wifi/

     install -d ${D}${sysconfdir}/initscripts/
     install -m 0755 ${S}/ifup-wlan ${D}${sysconfdir}/initscripts;
}

FILES_${PN} += "${sysconfdir}/"

FILES_SOLIBSDEV = ""
