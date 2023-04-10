# LIBFILEPARSER_SEARCH
# --------------
# Define AC_ARG_VAR (user variables), FILEPARSERLIB and FILEPARSERINC, 
# allowing the user to specify the location of the libfileparser toolkit 
# library and header file. If not specified, check for these files:
#
#   1. As a sub-project.
#   2. As a super-project (sibling to the current project).
#   3. As installed components on the system.
#
# If found, AC_SUBST FILEPARSER_LTLIB and FILEPARSER_INCLUDE variables with 
# values derived from FILEPARSERLIB and FILEPARSERINC user variables.
# FILEPARSERLIB and FILEPARSERINC are file locations, whereas FILEPARSER_LTLIB and 
# FILEPARSER_INCLUDE are linker and preprocessor command-line options.
#
# Author:   fulinux <fulinux@sina.com>
# Modified: 2014-09-12
# License:  AllPermissive
#
AC_DEFUN([LIBFILEPARSER_SEARCH],
[AC_ARG_VAR([FILEPARSERLIB], [The PATH wherein libfileparser.la can be found])
AC_ARG_VAR([FILEPARSERINC], [The PATH wherein *parser.h can be found])
dnl
# Ensure that both or neither FTK paths were specified.
if { test -n "$FILEPARSERLIB" && test -z "$FILEPARSERINC"; } || \
   { test -n "$FILEPARSERINC" && test -z "$FILEPARSERLIB"; } then
  AC_MSG_ERROR([Specify both FILEPARSERINC and FILEPARSERLIB, or neither.])
fi 

# Not specified? Check for FILEPARSER in standard places.
if test -z "$FILEPARSERLIB"; then
  # Check for libfileparser tool kit as a sub-project.
  if test -d "$srcdir/fileparser"; then
    AC_CONFIG_SUBDIRS([fileparser])
    FILEPARSERINC='$(top_srcdir)/fileparser/src'
    FILEPARSERLIB='$(top_builddir)/fileparser/src'
  else
    # Check for libfileparser tool kit as a super-project.
    if test -d "$srcdir/../fileparser"; then
      FILEPARSERINC='$(top_srcdir)/../fileparser/src'
      FILEPARSERLIB='$(top_builddir)/../fileparser/src'
    fi
  fi
fi

# Still empty? Check for *installed* libFILEPARSER tool kit.
if test -z "$FILEPARSERLIB"; then
  AC_CHECK_LIB([fileparser], [parse_ini_file], 
    [AC_CHECK_HEADERS([iniparser.h])
     LIBS="-lfileparser $LIBS"],
    [AC_MSG_ERROR([No libfileparser Toolkit found. Terminating.])])
fi

# AC_SUBST command line variables from FILEPARSERLIB and FILEPARSERINC.
if test -n "$FILEPARSERLIB"; then
  AC_SUBST([FILEPARSER_LTLIB], ["$FILEPARSERLIB/libfileparser.la"])
  AC_SUBST([FILEPARSER_INCLUDE], ["-I$FILEPARSERINC"])
fi[]dnl
])# LIBFILEPARSER_SEARCH
