#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

inherit qcommon qprebuilt qlicense

DESCRIPTION = "Quectel Lwm2m"

DEPENDS = "glib-2.0 ql-wifi ql-mcm-api ql-power ql-factory-reset ql-ota curl qlrild"

SRC_DIR = "${WORKSPACE}/quectel-app/lwm2m/"
S = "${WORKDIR}/quectel-app/lwm2m/"

PR = "r3"
FILES_${PN} += "${sysconfdir}"
FILES_${PN} += "${systemd_unitdir}/system/"
do_install_append() {
   if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
	install -d ${D}${sysconfdir}/initscripts
     	install -m 755 ${SRC_DIR}/src/start_lwm2mclient ${D}${sysconfdir}/initscripts/start_lwm2mclient

	install -d ${D}${systemd_unitdir}/system
	install -m 0644 ${SRC_DIR}//lwm2mclient.service ${D}${systemd_unitdir}/system/lwm2mclient.service

	install -d ${D}${sysconfdir}/
    install -m 0644 ${SRC_DIR}/lwm2m_log.conf ${D}${sysconfdir}/lwm2m_log.conf

	install -d ${D}${sysconfdir}/
    install -m 0644 ${SRC_DIR}/lwm2m.conf ${D}${sysconfdir}/lwm2m.conf

	install -d ${D}${systemd_unitdir}/system/multi-user.target.wants/
	ln -sf ${systemd_unitdir}/system/lwm2mclient.service  ${D}${systemd_unitdir}/system/multi-user.target.wants/lwm2mclient.service
  fi
}
do_package_qa() {
}