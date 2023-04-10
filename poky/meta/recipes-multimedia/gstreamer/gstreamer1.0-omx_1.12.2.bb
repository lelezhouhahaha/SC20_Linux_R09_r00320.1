include gstreamer1.0-omx.inc

LIC_FILES_CHKSUM = "file://COPYING;md5=4fbd65380cdd255951079008b364516c \
                    file://omx/gstomx.h;beginline=1;endline=21;md5=5c8e1fca32704488e76d2ba9ddfa935f"

SRC_URI = "http://gstreamer.freedesktop.org/src/gst-omx/gst-omx-${PV}.tar.xz \
           file://0001-SC20_Linux-Les-integrated-openmax-for-H264-decoder.patch \
           file://0001-SC20_R09_linux-Chapin-Multimedia-gstreamer-omx-h264e.patch \
           file://0001-SC20_R09_linux-Chapin-Multimedia-1-omxh264enc-2.patch \
           file://0001-SC20_R09_linux-Chapin-Multimedia-1-2-h264.patch \
           file://0001-SC20_R09_linux-Chapin-Multimedia-1-avc-reconfigure-e.patch \
           file://0001-SC20_Linux_R09-Chapin-GST-OMX-Index-OMX_IndexConfigV.patch"
           

SRC_URI[md5sum] = "4a1404a20b72e4ab6e826500218ec308"
SRC_URI[sha256sum] = "1b22398f45a027e977d2b5309625ec91cdcaf0da8751cbc7f596d639a45ba298"

S = "${WORKDIR}/gst-omx-${PV}"
