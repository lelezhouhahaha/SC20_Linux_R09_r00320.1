#
# Yocto Project Development Manual.
#

SUMMARY = "qlrild"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESPATH =+ "${WORKSPACE}/quectel-core/:"
SRC_URI = "file://qlrild"

SRC_DIR = "${WORKSPACE}/quectel-core/qlrild"
S = "${WORKDIR}/qlrild"

inherit distutils-base pkgconfig update-rc.d

INITSCRIPT_NAME= "start_qlrild_le"
INITSCRIPT_PARAMS = "start 47 2 3 4 5 . stop 57 0 1 6 ."
DEPENDS += "qcrild"
DEPENDS += "binder"
DEPENDS += "liblog"
DEPENDS += "libcutils"
DEPENDS += "libutils"
DEPENDS += "ql-misc"
DEPENDS += "nanopb"

do_compile() {
}

do_install() {
     install -d ${D}${bindir}
     install -m 0755 ${S}/usr/bin/* ${D}${bindir}

     install -d ${D}${libdir}
     install -m 0644 ${S}/usr/lib/* ${D}${libdir}

     install -d ${D}${includedir}
     install -d ${D}${includedir}/ims
     install -d ${D}${includedir}/gnss
     install -d ${D}${includedir}/smspdu
     install -m 0644 ${S}/usr/include/*.h ${D}${includedir}
     install -m 0644 ${S}/usr/include/ims/*.h ${D}${includedir}/ims/
     install -m 0644 ${S}/usr/include/gnss/*.h ${D}${includedir}/gnss/
     install -m 0644 ${S}/usr/include/smspdu/*.h ${D}${includedir}/smspdu/

     install -d ${D}${sysconfdir}/init.d
     install -m 0644 ${S}/etc/init.d/* ${D}${sysconfdir}/init.d/

     install -d ${D}${systemd_unitdir}/system/
     install -m 0644 ${S}/lib/systemd/system/qlrild.service ${D}${systemd_unitdir}/system/

     install -d ${D}${systemd_unitdir}/system/multi-user.target.wants
     ln -sf ${systemd_unitdir}/system/qlrild.service \
     ${D}${systemd_unitdir}/system/multi-user.target.wants/qlrild.service
}

do_package_qa() {
}

FILES_${PN} += "${systemd_unitdir}/*"
FILES_${PN} += "${libdir}/"
FILES_${PN} += "${bindir}/"
FILES_${PN} += "${includedir}/"
FILES_${PN} += "${sysconfdir}"
PACKAGE_STRIP = "no"
INSANE_SKIP_${PN} += "already-stripped"
