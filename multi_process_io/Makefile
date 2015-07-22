include ../config.make

LIBKMR=$(KMRDIR)/lib/libkmr.a

PRGS=kmr_memory ideal_memory less_locality_memory
OBJS=kmr_memory.o ideal_memory.o less_locality_memory.o

.PHONY: all clean distclean

all: $(PRGS)

.c.o:
	$(CC) $(DEFS) $(CFLAGS) -c $<

kmr_memory: kmr_memory.o
	$(CC) $(LDFLAGS) -o $@ $< $(LIBKMR)

ideal_memory: ideal_memory.o
	$(CC) $(LDFLAGS) -o $@ $< $(LIBKMR)

less_locality_memory: less_locality_memory.o
	$(CC) $(LDFLAGS) -o $@ $< $(LIBKMR)

$(OBJS): multi_process_io.h

clean:
	-rm -f $(PRGS) $(OBJS)
	-rm -f *~

distclean: clean
