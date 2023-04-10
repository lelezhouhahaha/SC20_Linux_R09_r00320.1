#                                               -*- Autoconf -*- 
# Process this file with autoconf to produce a configure script.

m4_define([version_major], [1])
m4_define([version_minor], [0])
m4_define([version_micro], [0])

m4_define([fileparser_release_status],
          [m4_if(m4_eval(version_minor % 2), [1], [snapshot], [release])])

m4_define([version],
          [version_major.version_minor.version_micro])

AC_PREREQ([2.63])
AC_INIT([fileparser], 
        [version], 
        [https://git.oschina.net/fulinux/fileparser/issues],
        [fileparser],
        [http://blog.csdn.net/fulinus])

AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([check-news foreign 1.11 -Wall -Wno-portability \
                  silent-rules tar-pax subdir-objects])

# AM_PROG_AR is not available in automake v0.11 but it's essential in v0.12.
# m4_ifdef([AM_PROG_AR], [AM_PROG_AR])
AM_PROG_AR

LT_PREREQ([2.0])
LT_INIT([dlopen])

AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SRCDIR([src/hexparser.c])
AC_CONFIG_SRCDIR([src/iniparser.c])
AC_CONFIG_HEADERS([config.h])

# Use the silent-rules feature when possible.
m4_ifndef([AM_SILENT_RULES], [m4_define([AM_SILENT_RULES],[])])
AM_SILENT_RULES([yes])

VERSION_MAJOR=version_major
VERSION_MINOR=version_minor
VERSION_MICRO=version_micro
VERSION=version
AC_SUBST(VERSION_MAJOR)
AC_SUBST(VERSION_MINOR)
AC_SUBST(VERSION_MICRO)
AC_SUBST(VERSION)

# Checks for programs.
#AC_PROG_CC
AC_PROG_CC_STDC
AC_PROG_INSTALL
AM_PROG_CC_C_O

# Checks for optional programs.
# AC_CHECK_PROGS([DOXYGEN], [doxygen])
# PROG_TRY_DOXYGEN

# Create Automake conditional based on the DOXYGEN variable
# AM_CONDITIONAL([HAVE_DOXYGEN], [test -n "$DOXYGEN"])
# AM_COND_IF([HAVE_DOXYGEN], [AC_CONFIG_FILES([docs/doxygen/doxyfile])])
# AS_IF([test -n "$DOXYGEN"], [AC_CONFIG_FILES([docs/doxygen/doxyfile])])

# Check whether to build docs / install man pages
# AC_CHECK_DOC_BUILD

# Checks for libraries.
# AC_SEARCH_LIBS([pthread_mutex_trylock], [pthread], [have_pthreads=yes])
# Check for pthreads

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdint.h stdlib.h string.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_TYPE_INT32_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT8_T

# My CFLAGS
my_CFLAGS="-O2 -Wall -Wno-parentheses \
    -fno-strict-aliasing -std=gnu99"
AC_SUBST([my_CFLAGS])

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([memmove memset strchr strtol])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 src/version.h
                 util/Makefile])

AC_OUTPUT

echo \
"-------------------------------------------------

${PACKAGE_NAME} Version ${PACKAGE_VERSION}

Prefix.........: ${prefix}
sysconfdir.....: ${sysconfdir}
libdir.........: ${libdir}
includedir.....: ${includedir}
Compiler.......: $CC $CFLAGS $CPPFLAGS
Linker.........: $LD $LDFLAGS $LIBS
Doxygen........: ${DOXYGEN:-NONE}
Debug Build....: $debug

Package features:

Now type 'make @<:@<target>@:>@'
where the optional <target> is:
all                - build all binaries
install            - install everything
uninstall          - uninstall everything
disclean           - clean useless everything

--------------------------------------------------"
