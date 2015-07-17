#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([KMR Benchark], [0.1.0], [s.takizawa@gmail.com])
AC_CONFIG_SRCDIR([Makefile])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([config.make])

# Checks for programs.
AC_PROG_CC
sinclude(ax_mpi.m4)
AX_MPI

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
CC=$MPICC
CFLAGS="$CFLAGS -std=gnu99 -Wall"
LIBS="$MPILIBS $LIBS"

# Checks for library functions.

AC_OUTPUT
