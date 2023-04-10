############################################################################
# FindQMI.txt
# Copyright (C) 2020 Peeta Chen peeta.chen@quectel.com
#
############################################################################

#
# - Find the QMI include file and library
#
#  QMI_FOUND - system has QMI
#  QMI_INCLUDE_DIRS - the QMI include directory
#  QMI_LIBRARIES - The libraries needed to use QMI

set(_QMI_ROOT_PATHS
	${CMAKE_INSTALL_PREFIX}
)

find_path(QMI_INCLUDE_DIRS
    NAMES qmi_port_defs.h
    HINTS _QMI_ROOT_PATHS
    PATH_SUFFIXES include qmi
)

if(QMI_INCLUDE_DIRS)
    set(HAVE_QMI_QMI_H 1)
    set(QMI_VERSION 1)
endif()

find_library(QMI_LIBRARIES
NAMES qmi
HINTS ${_QMI_ROOT_PATHS}
PATH_SUFFIXES bin lib
)


include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QMI
	DEFAULT_MSG
	QMI_INCLUDE_DIRS QMI_LIBRARIES HAVE_QMI_QMI_H QMI_VERSION
)

mark_as_advanced(QMI_INCLUDE_DIRS QMI_LIBRARIES HAVE_QMI_QMI_H QMI_VERSION)
