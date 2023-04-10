#
# Yocto Project Development Manual.
#
SUMMARY = "Sound Manager Project"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESPATH =+ "${WORKSPACE}/quectel-app/:"
SRC_URI = "file://sound-manager/"

SRC_DIR = "${WORKSPACE}/quectel-app/sound-manager"
S = "${WORKDIR}/sound-manager"

INSANE_SKIP_${PN} += "dev-deps"

inherit cmake distutils-base pkgconfig

DEPENDS = "cmake (>= 2.6.3)"
DEPENDS += "alsa-intf-msm8909"
DEPENDS += "ql-mcm-api"
DEPENDS += "qlrild"
DEPENDS += "ql-misc"
DEPENDS += "liblog"
DEPENDS += "libcutils"
DEPENDS += "fileparser"
DEPENDS += "libqaudio"

#EXTRA_OECMAKE +="-DBUILD_SHARED_LIBS=OFF"
EXTRA_OECMAKE_append += " -DCMAKE_SKIP_RPATH=ON"
EXTRA_OECMAKE_append += " -DCMAKE_INSTALL_DO_STRIP=1"

BBCLASSEXTEND = "native nativesdk"

FILES_SOLIBSDEV = ""

do_install_append() {
    install -d ${D}/etc/misc/soundlib/
    install -m 0644 ${S}/soundlib/* ${D}/etc/misc/soundlib/

    install -d ${D}${systemd_unitdir}/system
    install -m 0644 ${S}/sound-manager.service.in ${D}${systemd_unitdir}/system/sound-manager.service

    install -d ${D}${systemd_unitdir}/system/multi-user.target.wants/
    ln -sf ${systemd_unitdir}/system/sound-manager.service \
        ${D}${systemd_unitdir}/system/multi-user.target.wants/sound-manager.service

    install -d ${D}${sysconfdir}/
    install -m 0644 ${S}/sound_manager.ini ${D}${sysconfdir}/sound_manager.ini
}

FILES_${PN} = "/usr/bin/*" 
FILES_${PN} += "${systemd_unitdir}/*" 
FILES_${PN} += "${sysconfdir}/*" 
