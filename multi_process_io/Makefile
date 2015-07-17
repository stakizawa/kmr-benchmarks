include ../config.make

LIBKMR=$(KMRDIR)/lib/libkmr.a

PRGS=kmr_memory
OBJS=kmr_memory.o

.PHONY: all clean distclean

all: $(PRGS)

.c.o:
	$(CC) $(DEFS) $(CFLAGS) -c $<

kmr_memory: kmr_memory.o
	$(CC) $(LDFLAGS) -o $@ $< $(LIBKMR)

clean:
	-rm -f $(PRGS) $(OBJS)
	-rm -f *~

distclean: clean
