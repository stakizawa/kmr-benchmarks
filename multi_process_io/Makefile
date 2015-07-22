include ../config.make

LIBKMR=$(KMRDIR)/lib/libkmr.a

PRGS=kmr_memory ideal less_locality
OBJS=kmr_memory.o ideal.o less_locality.o

.PHONY: all clean distclean

all: $(PRGS)

.c.o:
	$(CC) $(DEFS) $(CFLAGS) -c $<

kmr_memory: kmr_memory.o multi_process_io.h
	$(CC) $(LDFLAGS) -o $@ $< $(LIBKMR)

ideal: ideal.o multi_process_io.h
	$(CC) $(LDFLAGS) -o $@ $< $(LIBKMR)

less_locality: less_locality.o multi_process_io.h
	$(CC) $(LDFLAGS) -o $@ $< $(LIBKMR)

clean:
	-rm -f $(PRGS) $(OBJS)
	-rm -f *~

distclean: clean
