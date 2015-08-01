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

# Checks compiler
AC_PROG_CC
! (type fccpx 1>/dev/null 2>/dev/null); fujitsu_env=$?
if test "$fujitsu_env" -eq "1"; then
   CC=fccpx
   LD=ldpx
   CFLAGS=""
fi

# Checks debug option
AC_ARG_ENABLE([debug],
	      [AS_HELP_STRING([--enable-debug],
			      [turn on debugging [default=no]])],
	      [], [enable_debug=no])
if test "$enable_debug" = "yes"; then
   CFLAGS="-DDEBUG $CFLAGS"
fi

# Checks MPI
AC_LANG(C)
AX_MPI

# Checks OpenMP
AC_ARG_ENABLE([openmp],
	      [AS_HELP_STRING([--enable-openmp],
			      [turn on openmp [default=yes]])],
	      [], [enable_openmp=yes])
if test "$enable_openmp" != "no"; then
	AX_OPENMP([OPENMP_CFLAGS=$ax_cv_c_openmp])
fi
AC_SUBST(OPENMP_CFLAGS)

# Checks KMR
AC_ARG_WITH([kmr],
	    [AS_HELP_STRING([--with-kmr=PATH],
			    [set path for kmr [default=/usr/local]])],
	    [KMRDIR=$withval], [KMRDIR=/usr/local])
AC_SUBST(KMRDIR)

# Sets compiler flags
CFLAGS="$CFLAGS -I $KMRDIR/include $OPENMP_CFLAGS"
case "$CC" in
gcc)
    CFLAGS="-std=gnu99 -Wall $CFLAGS"
    ;;
fccpx)
    CFLAGS="-Kfast,preex -Xg -std=gnu99 $CFLAGS"
    ;;
esac

CC=$MPICC
LDFLAGS="$OPENMP_CFLAGS $LDFLAGS"
LIBS="$MPILIBS $LIBS"

AC_OUTPUT
