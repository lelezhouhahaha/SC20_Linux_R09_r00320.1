FILESEXTRAPATHS_prepend := "${THISDIR}/${PN}-${PV}:"
SRC_URI += "\
file://dhcpcd_iface_info.patch "
SRC_URI += "file://dhcpcd_resolv_info.patch"
