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
AC_ARG_ENABLE([debug],
	      [AS_HELP_STRING([--enable-debug],
			      [turn on debugging [default=no]])],
	      [], [enable_debug=no])
if test "$enable_debug" = "yes"; then
	CFLAGS="-DDEBUG $CFLAGS"
fi

# Checks for libraries.
AX_MPI
AC_ARG_ENABLE([openmp],
	      [AS_HELP_STRING([--enable-openmp],
			      [turn on openmp [default=yes]])],
	      [], [enable_openmp=yes])
if test "$enable_openmp" != "no"; then
	AX_OPENMP
fi
AC_SUBST(OPENMP_CFLAGS)
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

# Checks for library functions.

AC_OUTPUT
