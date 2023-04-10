inherit qcommon qprebuilt qlicense

DESCRIPTION = "Quectel default NV settings"
PR = "r7"

SRC_DIR = "${WORKSPACE}/quectel-app/ql-nv-settings/"

S = "${WORKDIR}/quectel-app/ql-nv-settings/"

DEPENDS = "ql-misc"

do_install_append() {
    install -d ${D}${systemd_unitdir}/system
    install -m 0644 ${S}/ql_nv_settings.service.in ${D}${systemd_unitdir}/system/ql_nv_settings.service

    install -d ${D}${systemd_unitdir}/system/multi-user.target.wants/
    ln -sf ${systemd_unitdir}/system/ql_nv_settings.service \
    ${D}${systemd_unitdir}/system/multi-user.target.wants/ql_nv_settings.service
}

FILES_${PN} += "${systemd_unitdir}/*"
FILES_${PN} += "/etc/*"
FILES_${PN} += "${bindir}/*"
