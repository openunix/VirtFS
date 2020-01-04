#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.60])
AC_INIT([virtfs], [0.0.1], [steve.shuo.feng@gmail.com])
AC_CONFIG_SRCDIR([lib/virtfs.c])
AC_CONFIG_HEADERS([include/config.h])

AM_INIT_AUTOMAKE

# SLES 11SP2 (automake-1.10.1) doesn't have this
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL
AM_PROG_CC_C_O

AC_GNU_SOURCE

AC_PROG_LIBTOOL

# Checks for libraries.
AC_CHECK_LIB([readline], [readline], [], AC_MSG_ERROR([You need readline to run.]))

# Checks for header files.

# We won't checks for types because fuse and tirpc will require that.

# Checks for structures

# Check for compiler characteristics.

# Checks for library functions.

# Check for system service
AC_SYS_LARGEFILE

AC_CONFIG_FILES([Makefile
                 include/Makefile
                 lib/Makefile
                ])
AC_OUTPUT
