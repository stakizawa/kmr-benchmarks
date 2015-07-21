include ../config.make

LIBKMR=$(KMRDIR)/lib/libkmr.a

PRGS=kmr_memory ideal
OBJS=kmr_memory.o ideal.o

.PHONY: all clean distclean

all: $(PRGS)

.c.o:
	$(CC) $(DEFS) $(CFLAGS) -c $<

kmr_memory: kmr_memory.o multi_process_io.h
	$(CC) $(LDFLAGS) -o $@ $< $(LIBKMR)

ideal: ideal.o multi_process_io.h
	$(CC) $(LDFLAGS) -o $@ $< $(LIBKMR)

clean:
	-rm -f $(PRGS) $(OBJS)
	-rm -f *~

distclean: clean
