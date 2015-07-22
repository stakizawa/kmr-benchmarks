include ../config.make

LIBKMR=$(KMRDIR)/lib/libkmr.a

PRGS=kmr_memory ideal_memory less_locality_memory \
     kmr_file ideal_file
OBJS=kmr_memory.o ideal_memory.o less_locality_memory.o \
     kmr_file.o ideal_file.o

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

kmr_file: kmr_file.o
	$(CC) $(LDFLAGS) -o $@ $< $(LIBKMR)

ideal_file: ideal_file.o
	$(CC) $(LDFLAGS) -o $@ $< $(LIBKMR)

$(OBJS): multi_process_io.h

clean:
	-rm -f $(PRGS) $(OBJS)
	-rm -f *~

distclean: clean
