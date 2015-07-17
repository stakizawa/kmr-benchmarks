.PHONY: all clean distclean

all:
	cd multi_process_io; make all

configure: configure.ac m4/ax_mpi.m4 config.make.in
	autoconf
	autoheader
	-rm -fr autom4te.cache

clean:
	cd multi_process_io; make clean
	-rm *~

distclean:
	cd multi_process_io; make distclean
	-rm -f config.status config.log config.h config.make
