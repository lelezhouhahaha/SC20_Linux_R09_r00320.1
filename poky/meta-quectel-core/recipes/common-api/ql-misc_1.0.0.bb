#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

SUMMARY = "Simple ql_misc library"
LICENSE = "MIT"
LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"

FILESPATH =+ "${WORKSPACE}/quectel-core/:"
SRC_URI = "file://ql-misc/"

SRC_DIR = "${WORKSPACE}/quectel-core/ql-misc"
S = "${WORKDIR}/ql-misc"

#inherit cmake

PACKAGES = "${PN}"
FILES_${PN} += "${libdir}/" 
FILES_${PN}-dbg += "${libdir}/.debug/*"
FILES_${PN} += "${includedir}/*"
FILES_SOLIBSDEV = ""
INSANE_SKIP_${PN} = "dev-so"

DEPENDS = "libcutils"
#DEPENDS = "cmake (>= 2.6.3)"
#EXTRA_OECMAKE +="-DBUILD_SHARED_LIBS=OFF"
#EXTRA_OECMAKE_append += " -DCMAKE_SKIP_RPATH=ON"
#EXTRA_OECMAKE_append += " -DCMAKE_INSTALL_DO_STRIP=1"

do_install() {
    install -d ${D}${includedir}
    install -m 0444 ${S}/include/*.h -D ${D}${includedir}/

    install -d ${D}${libdir}
    if [ -f "${S}/lib/libql_misc.a" ]; then
    install -m 0644 ${S}/lib/*.a -D ${D}${libdir}/
    fi
    if [ -f "${S}/lib/libql_misc.so" ]; then
    install -m 0644 ${S}/lib/*.so -D ${D}${libdir}/
    fi
    #chrpath -d ${D}${bindir}/debug-test
}

do_package_qa() {
}
