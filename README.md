KMR Benchmarks
==============

This project provides various benchmark programs for KMR performance
measurement.

KMR: http://mt.aics.riken.jp/kmr

Compile
-------

KMR Benchmarks depend on a C compiler that supports C99 standard,
a standard MPI library that supports MPI 2.2 and KMR.  Though KMR
Benchmarks may work with any combinations of C compiler and MPI
implementation, I tested on the following environments.

* CentOS 6.5 Linux x86_64, GCC 4.4.7, OpenMPI 1.8.3
* K Computer/FX10, Fujitsu Compiler, Fujitsu MPI (latest stable)

KMR Benchmarks can be compiled by just typing 'configure', 'make'.
However path to an installed KMR should be given as a configure option.

    $ ./configure --with-kmr=KMRPATH
    $ make

If you want to disable OpenMP support, give '--disable-openmp' option
to the configure script.  This requires that the KMR is also build
without OpenMP support.
