inherit autotools pkgconfig

DESCRIPTION = "ALSA Framework Library"
LICENSE = "Apache-2.0"
PR = "r5"
LIC_FILES_CHKSUM = "file://${COREBASE}/meta/files/common-licenses/${LICENSE};md5=89aea4e17d99a7cacdbeed46a0096b10"
DEPENDS = "acdbloader glib-2.0"

FILESPATH =+ "${WORKSPACE}:"
SRC_URI = "file://quectel-app/qlalsa-audio-test"

S = "${WORKDIR}/quectel-app/qlalsa-audio-test"

DEPENDS = "common dsutils glib-2.0 alsa-intf-msm8909"

EXTRA_OECONF += "--prefix=/etc \
                 --with-sanitized-headers=${STAGING_KERNEL_BUILDDIR}/usr/include \
                 --with-glib \
                 --with-acdb "

FILES_${PN} += "${prefix}/snd_soc_msm/*"

do_configure[depends] += "virtual/kernel:do_shared_workdir"

do_install_append() {
    install -d ${D}${bindir}
    install -m 0755 qlplay ${D}${bindir}
    install -m 0755 qlmix ${D}${bindir}
    install -m 0755 qlrec ${D}${bindir}
}
