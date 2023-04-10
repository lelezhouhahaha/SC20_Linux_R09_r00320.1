project(ql-gnss-test C)
cmake_minimum_required(VERSION 2.6.3)
#set ( CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake_admin )

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# package version
find_package (PkgConfig REQUIRED)
find_package (QMI REQUIRED)

find_library(QMI_LIBS NAMES qmi PATH_SUFFIXES lib)
find_library(QMI_COMMON_SO_LIBS NAMES qmi_common_so PATH_SUFFIXES lib)
find_library(QMI_ENCDEC_LIBS NAMES qmi_encdec PATH_SUFFIXES lib)
find_library(QMI_CCI_LIBS NAMES qmi_cci PATH_SUFFIXES lib)
find_library(QMI_CSI_LIBS NAMES qmi_csi PATH_SUFFIXES lib)
find_library(QMI_SAP_LIBS NAMES qmi_sap PATH_SUFFIXES lib)
find_library(QMI_SERVICE_LIBS NAMES qmiservices PATH_SUFFIXES lib)
find_library(CUTILS_LIBS NAMES cutils PATH_SUFFIXES lib)

add_subdirectory(src)
