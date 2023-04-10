DESCRIPTION = "WCNSS platform"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/BSD;md5=3775480a712fc46a69647678acb234cb"
LICENSE = "BSD"

PR = "r1"
FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://qcom-opensource/wlan/prima/firmware_bin \
           file://set_wcnss_mode"
SRC_URI += "file://wcnss_wlan.service"
SRC_URI += "file://device/qcom/wlan/${SOC_FAMILY}/WCNSS_qcom_cfg.ini"
SRC_URI += "file://device/qcom/wlan/${SOC_FAMILY}/WCNSS_qcom_wlan_nv.bin"
SRC_URI += "file://device/qcom/wlan/${SOC_FAMILY}/WCNSS_wlan_dictionary.dat"

S = "${WORKDIR}/qcom-opensource/wlan/firmware_bin"

inherit systemd
inherit update-rc.d

do_install() {

	if ${@bb.utils.contains('DISTRO_FEATURES', 'systemd', 'true', 'false', d)}; then
                install -d ${D}/etc/initscripts/
                install "${WORKDIR}"/set_wcnss_mode ${D}/etc/initscripts/set_wcnss_mode
		install -d ${D}/etc/systemd/system/
		install -m 0644 ${WORKDIR}/wcnss_wlan.service -D ${D}/etc/systemd/system/wcnss_wlan.service
	        install -d ${D}/etc/systemd/system/multi-user.target.wants/
	        install -d ${D}/etc/systemd/system/ffbm.target.wants/
		# enable the service for multi-user.target
		ln -sf /etc/systemd/system/wcnss_wlan.service \
		${D}/etc/systemd/system/multi-user.target.wants/wcnss_wlan.service
		# enable the service for ffbm.target, ffbm is used for factory
		ln -sf /etc/systemd/system/wcnss_wlan.service \
		${D}/etc/systemd/system/ffbm.target.wants/wcnss_wlan.service
        else
               install -d ${D}/etc
               install -d ${D}/etc/init.d
               install "${WORKDIR}"/set_wcnss_mode ${D}/etc/init.d
	fi

    install -d ${D}/lib/firmware/wlan/prima/
    install -d ${D}/persist/

    if [ -e "${WORKDIR}/device/qcom/wlan/${SOC_FAMILY}/WCNSS_qcom_cfg.ini" ];then
	    install -m 0644 ${WORKDIR}/device/qcom/wlan/${SOC_FAMILY}/WCNSS_qcom_cfg.ini ${D}/lib/firmware/wlan/prima/WCNSS_qcom_cfg.ini
    elif [ -e "${WORKSPACE}/android_compat/device/qcom/${SOC_FAMILY}/WCNSS_qcom_cfg.ini" ];then
	    cp -pP ${WORKSPACE}/android_compat/device/qcom/${SOC_FAMILY}/WCNSS_qcom_cfg.ini ${D}/lib/firmware/wlan/prima
    fi
    if [ -e "${WORKDIR}/device/qcom/wlan/${SOC_FAMILY}/WCNSS_qcom_wlan_nv.bin" ];then
	    install -m 0644 "${WORKDIR}/device/qcom/wlan/${SOC_FAMILY}/WCNSS_qcom_wlan_nv.bin" ${D}/persist/WCNSS_qcom_wlan_nv.bin
    fi
    if [ -e "${WORKDIR}/device/qcom/wlan/${SOC_FAMILY}/WCNSS_wlan_dictionary.dat" ]; then
	    install -m 0644 "${WORKDIR}/device/qcom/wlan/${SOC_FAMILY}/WCNSS_wlan_dictionary.dat" ${D}/persist/WCNSS_wlan_dictionary.dat
    fi
}
do_install_append() {
   install -d ${D}/lib/firmware/wlan/prima
   ln -s /persist/WCNSS_qcom_wlan_nv.bin ${D}/lib/firmware/wlan/prima/WCNSS_qcom_wlan_nv.bin
   ln -s /persist/WCNSS_wlan_dictionary.dat ${D}/lib/firmware/wlan/prima/WCNSS_wlan_dictionary.dat
}
INITSCRIPT_NAME = "set_wcnss_mode"
INITSCRIPT_PARAMS = "start 60 2 3 4 5 . stop 20 0 1 6 ."

FILES_${PN} = "/lib/firmware/*"
FILES_${PN} += "/etc/*"
FILES_${PN} += "/lib/firmware/wlan/prima/*"
FILES_${PN} += "/persist/*"

