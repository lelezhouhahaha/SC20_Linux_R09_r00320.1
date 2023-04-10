#
# Yocto Project Development Manual.
#

SUMMARY = "qcrild"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESPATH =+ "${WORKSPACE}/quectel-core/:"
SRC_URI = "file://qcrild/"

SRC_DIR = "${WORKSPACE}/quectel-core/qcrild"
S = "${WORKDIR}/qcrild"

DEPENDS = "binder"
DEPENDS += "libcutils libutils"
DEPENDS += "qmi qmi-framework"
DEPENDS += "protobuf protobuf-native nanopb-native python-protobuf-native"
DEPENDS_append_class-native = " python-protobuf-native"

INSANE_SKIP_${PN} += "dev-deps"

do_compile() {
}

do_install() {
     install -d ${D}${bindir}
     install -m 0755 ${S}/usr/bin/* ${D}${bindir}

     install -d ${D}${libdir}
     install -m 0644 ${S}/usr/lib/* ${D}${libdir}

     install -d ${D}${includedir}
     install -d ${D}${includedir}/telephony
     install -m 0644 ${S}/usr/include/*.h ${D}${includedir}
     install -m 0644 ${S}/usr/include/telephony/*.h ${D}${includedir}/telephony/

     install -d ${D}${sysconfdir}/init.d
     install -m 0644 ${S}/etc/init.d/* ${D}${sysconfdir}/init.d/

     install -d ${D}${systemd_unitdir}/system/
     install -d ${D}${systemd_unitdir}/system/multi-user.target.wants

	 if [ -f '${S}/lib/systemd/system/pm-service.service' ]; then
		install -m 0644 ${S}/lib/systemd/system/pm-service.service ${D}${systemd_unitdir}/system/
	 fi
     install -m 0644 ${S}/lib/systemd/system/qcrild.service ${D}${systemd_unitdir}/system/
     install -m 0644 ${S}/lib/systemd/system/qcrild2.service ${D}${systemd_unitdir}/system/

#     ln -sf ${systemd_unitdir}/system/pm-service.service \
#     ${D}${systemd_unitdir}/system/multi-user.target.wants/pm-service.service

     install -d ${D}/data/misc/radio/
}

do_package_qa() {
}

FILES_${PN} += "${systemd_unitdir}/*"
FILES_${PN} += "${libdir}/"
FILES_${PN} += "${bindir}/"
FILES_${PN} += "${includedir}/"
FILES_${PN} += "${sysconfdir}"
FILES_${PN} += "/data/misc/*"
PACKAGE_STRIP = "no"
INSANE_SKIP_${PN} += "already-stripped"
