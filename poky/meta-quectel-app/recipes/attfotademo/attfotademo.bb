#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

inherit qcommon qprebuilt qlicense

DESCRIPTION = "Quectel attfotademo"

DEPENDS = "glib-2.0 ql-wifi ql-mcm-api ql-power ql-factory-reset ql-ota curl"

SRC_DIR = "${WORKSPACE}/quectel-app/attfotademo/"
S = "${WORKDIR}/quectel-app/attfotademo/"

PR = "r3"
FILES_${PN} += "${sysconfdir}"
FILES_${PN} += "${systemd_unitdir}/system/"
do_install_append() {
   if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
	install -d ${D}${sysconfdir}/initscripts
     	install -m 755 ${SRC_DIR}src/start_attfotademo ${D}${sysconfdir}/initscripts/start_attfotademo

	install -d ${D}${systemd_unitdir}/system
	install -m 0644 ${SRC_DIR}attfotademo.service ${D}${systemd_unitdir}/system/attfotademo.service


	install -d ${D}${systemd_unitdir}/system/multi-user.target.wants/
	ln -sf ${systemd_unitdir}/system/attfotademo.service  ${D}${systemd_unitdir}/system/multi-user.target.wants/attfotademo.service
  fi
}

