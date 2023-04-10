#
# 
# Yocto Project Development Manual.
#
inherit qcommon qprebuilt qlicense

#SUMMARY = "sampleclient application"
#LICENSE = "MIT"
#LIC_FILES_CHKSUM = "file://${COMMON_LICENSE_DIR}/MIT;md5=0835ade698e0bcf8506ecda2f7b4f302"
DESCRIPTION = "Quectel sampleclient application"
PR = "r7"

#FILESPATH =+ "${WORKSPACE}:"


S = "${WORKDIR}/quectel-app/sampleclient"
SRC_DIR = "${WORKSPACE}/quectel-app/sampleclient/"

FILES_${PN} += "/system/lib"
FILES_${PN} += "/system/bin"
FILES_${PN} += "/vendor/lib"
FILES_${PN} += "/vendor/bin"

do_compile() {
}

do_install() {
	install -dm0777 ${D}/system/lib
	install -dm0777 ${D}/system/lib/vndk-sp
	install -dm0777 ${D}/vendor/lib
	install -dm0777 ${D}/vendor/bin
	install -dm0777 ${D}/system/bin


	install -m 0777 ${S}/system/lib/android.hidl.base@1.0.so -D ${D}/system/lib/android.hidl.base@1.0.so
	install -m 0777 ${S}/system/lib/ld-android.so -D ${D}/system/lib/ld-android.so
	install -m 0777 ${S}/system/lib/libbacktrace.so -D ${D}/system/lib/libbacktrace.so
	install -m 0777 ${S}/system/lib/libbase.so -D ${D}/system/lib/libbase.so
	install -m 0777 ${S}/system/lib/libbinder.so -D ${D}/system/lib/libbinder.so
	install -m 0777 ${S}/system/lib/libc.so -D ${D}/system/lib/libc.so
	install -m 0777 ${S}/system/lib/libc++.so -D ${D}/system/lib/libc++.so
	install -m 0777 ${S}/system/lib/libcutils.so -D ${D}/system/lib/libcutils.so
	install -m 0777 ${S}/system/lib/libdl.so -D ${D}/system/lib/libdl.so
	install -m 0777 ${S}/system/lib/libhidlbase.so -D ${D}/system/lib/libhidlbase.so
	install -m 0777 ${S}/system/lib/libhidltransport.so -D ${D}/system/lib/libhidltransport.so
	install -m 0777 ${S}/system/lib/libhwbinder.so -D ${D}/system/lib/libhwbinder.so
	install -m 0777 ${S}/system/lib/liblog.so -D ${D}/system/lib/liblog.so
	install -m 0777 ${S}/system/lib/liblzma.so -D ${D}/system/lib/liblzma.so
	install -m 0777 ${S}/system/lib/libm.so -D ${D}/system/lib/libm.so
	install -m 0777 ${S}/system/lib/libunwind.so -D ${D}/system/lib/libunwind.so
	install -m 0777 ${S}/system/lib/libutils.so -D ${D}/system/lib/libutils.so
	install -m 0777 ${S}/system/lib/libvndksupport.so -D ${D}/system/lib/libvndksupport.so

	install -m 0777 ${S}/system/lib/vndk-sp/android.hidl.base@1.0.so -D ${D}/system/lib/vndk-sp/android.hidl.base@1.0.so
	install -m 0777 ${S}/system/lib/vndk-sp/libbacktrace.so -D ${D}/system/lib/vndk-sp/libbacktrace.so
	install -m 0777 ${S}/system/lib/vndk-sp/libbase.so -D ${D}/system/lib/vndk-sp/libbase.so
	install -m 0777 ${S}/system/lib/vndk-sp/libc++.so -D ${D}/system/lib/vndk-sp/libc++.so
	install -m 0777 ${S}/system/lib/vndk-sp/libcutils.so -D ${D}/system/lib/vndk-sp/libcutils.so
	install -m 0777 ${S}/system/lib/vndk-sp/libhidlbase.so -D ${D}/system/lib/vndk-sp/libhidlbase.so
	install -m 0777 ${S}/system/lib/vndk-sp/libhidltransport.so -D ${D}/system/lib/vndk-sp/libhidltransport.so
	install -m 0777 ${S}/system/lib/vndk-sp/libhwbinder.so -D ${D}/system/lib/vndk-sp/libhwbinder.so
	install -m 0777 ${S}/system/lib/vndk-sp/liblzma.so -D ${D}/system/lib/vndk-sp/liblzma.so
	install -m 0777 ${S}/system/lib/vndk-sp/libunwind.so -D ${D}/system/lib/vndk-sp/libunwind.so
	install -m 0777 ${S}/system/lib/vndk-sp/libutils.so -D ${D}/system/lib/vndk-sp/libutils.so

	install -m 0777 ${S}/vendor/lib/android.hidl.base@1.0.so -D ${D}/vendor/lib/android.hidl.base@1.0.so
	install -m 0777 ${S}/vendor/lib/libdiag.so -D ${D}/vendor/lib/libdiag.so
	install -m 0777 ${S}/vendor/lib/libdrmfs.so -D ${D}/vendor/lib/libdrmfs.so
	install -m 0777 ${S}/vendor/lib/libdrmtime.so -D ${D}/vendor/lib/libdrmtime.so
	install -m 0777 ${S}/vendor/lib/libGPreqcancel.so -D ${D}/vendor/lib/libGPreqcancel.so
	install -m 0777 ${S}/vendor/lib/libqisl.so -D ${D}/vendor/lib/libqisl.so
	install -m 0777 ${S}/vendor/lib/libQSEEComAPI.so -D ${D}/vendor/lib/libQSEEComAPI.so
	install -m 0777 ${S}/vendor/lib/librpmb.so -D ${D}/vendor/lib/librpmb.so
	install -m 0777 ${S}/vendor/lib/libsecureui.so -D ${D}/vendor/lib/libsecureui.so
	install -m 0777 ${S}/vendor/lib/libsecureui_svcsock.so -D ${D}/vendor/lib/libsecureui_svcsock.so
	install -m 0777 ${S}/vendor/lib/libSecureUILib.so -D ${D}/vendor/lib/libSecureUILib.so
	install -m 0777 ${S}/vendor/lib/libssd.so -D ${D}/vendor/lib/libssd.so
	install -m 0777 ${S}/vendor/lib/libStDrvInt.so -D ${D}/vendor/lib/libStDrvInt.so
	install -m 0777 ${S}/vendor/lib/vendor.qti.hardware.tui_comm@1.0_vendor.so -D ${D}/vendor/lib/vendor.qti.hardware.tui_comm@1.0_vendor.so

	install -m 0777 ${S}/vendor/bin/qseecom_sample_client -D ${D}/vendor/bin/qseecom_sample_client
	install -m 0777 ${S}/vendor/bin/qseecomd -D ${D}/vendor/bin/qseecomd

	install -m 0777 ${S}/system/bin/linker -D ${D}/system/bin/linker
}

do_package_qa() {
}
