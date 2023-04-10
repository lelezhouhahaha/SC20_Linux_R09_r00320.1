inherit autotools-brokensep pkgconfig

DESCRIPTION = "Build Android libion"
HOMEPAGE = "http://developer.android.com/"
LICENSE = "Apache-2.0"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/\
${LICENSE};md5=89aea4e17d99a7cacdbeed46a0096b10"

PR = "r1"

FILESPATH =+ "${WORKSPACE}/system/core/:"
SRC_URI   = "file://libion"

S = "${WORKDIR}/libion"
DEPENDS += "virtual/kernel liblog"

EXTRA_OECONF += " --disable-static"
EXTRA_OECONF += "${@bb.utils.contains('PREFERRED_VERSION_linux-msm', '4.14', '', '--with-legacyion', d)}"
EXTRA_OECONF += "--with-sanitized-headers=${STAGING_KERNEL_BUILDDIR}/usr/include"

PACKAGES +="${PN}-test-bin"

FILES_${PN}     = "${libdir}/pkgconfig/* ${libdir}/* ${sysconfdir}/*"
FILES_${PN}-test-bin = "${base_bindir}/*"
do_configure[depends] += "virtual/kernel:do_shared_workdir"
