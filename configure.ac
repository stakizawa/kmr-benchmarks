#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.63])
AC_INIT([KMR Benchark], [0.1.0], [s.takizawa@gmail.com])
AC_CONFIG_SRCDIR([Makefile])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_FILES([config.make])
AC_CONFIG_MACRO_DIR([m4/])
m4_include([m4/ax_mpi.m4])
m4_include([m4/ax_openmp.m4])

# Checks for programs.
AC_PROG_CC
AX_MPI
AX_OPENMP

# Checks for libraries.
AC_ARG_WITH([kmr],
	    [AS_HELP_STRING([--with-kmr=PATH],
			    [set path for kmr [default=/usr/local]])],
	    [KMRDIR=$withval], [KMRDIR=/usr/local])
AC_SUBST(KMRDIR)

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.
CC=$MPICC
CFLAGS="$CFLAGS -std=gnu99 -Wall -I $KMRDIR/include $OPENMP_CFLAGS"
LDFLAGS="$OPENMP_CFLAGS $LDFLAGS"
LIBS="$MPILIBS $LIBS"
AC_SUBST(OPENMP_CFLAGS)

# Checks for library functions.

AC_OUTPUT
