#
# This file was derived from the 'Hello World!' example recipe in the
# Yocto Project Development Manual.
#

inherit qcommon qprebuilt qlicense

DESCRIPTION = "Quectel mbedtls"


DEPENDS = "glib-2.0"

SRC_DIR = "${WORKSPACE}/quectel-core/ql-mbedtls/"
S = "${WORKDIR}/quectel-core/ql-mbedtls/"
