inherit autotools 
SUMMARY = "VisualOn AAC encoder library"
DESCRIPTION = "This library contains an encoder implementation of the \
                Advanced Audio Coding (AAC) audio codec. The library is \
                based on a codec implementation by VisualOn as part of the \
                Stagefright framework from the Google Android project."
SECTION = "libs"

LICENSE = "LGPL-2.0+"

LIC_FILES_CHKSUM = "file://COPYING;md5=dd2c2486aca02190153cf399e508c7e7"

SRC_URI = "git://github.com/mstorsjo/vo-aacenc.git;protocol=https;branch=master"

SRCREV = "a277487e051e92e99a532294eed3c673f4d879f2"

S = "${WORKDIR}/git"
