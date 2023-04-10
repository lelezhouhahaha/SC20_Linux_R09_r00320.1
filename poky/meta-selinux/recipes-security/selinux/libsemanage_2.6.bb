include selinux_20161014.inc
include ${BPN}.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=a6f89e2100d9b6cdffcea4f398e37343"

SRC_URI[md5sum] = "666a48c4058c07f2b07ede9eaf210c5f"
SRC_URI[sha256sum] = "4f81541047290b751f2ffb926fcd381c186f22db18d9fe671b0b4a6a54e8cfce"

SRC_URI += "\
	file://libsemanage-Fix-execve-segfaults-on-Ubuntu.patch \
	file://libsemanage-fix-path-len-limit.patch \
	file://libsemanage-fix-path-nologin.patch \
	file://libsemanage-drop-Wno-unused-but-set-variable.patch \
	file://libsemanage-define-FD_CLOEXEC-as-necessary.patch;striplevel=2 \
	file://libsemanage-allow-to-disable-audit-support.patch \
	file://libsemanage-disable-expand-check-on-policy-load.patch \
	file://0001-src-Makefile-fix-includedir-in-libselinux.pc.patch \
	"
FILES_${PN} += "/usr/libexec"
