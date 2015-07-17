include ../config.make

PRGS=kmr_memory
OBJS=kmr_memory.o

.PHONY: all clean distclean

all: $(PRGS)

.c.o:
	$(CC) $(DEFS) $(CFLAGS) -c $<

kmr_memory: kmr_memory.o
	$(CC) -o $@ $<

clean:
	-rm -f $(PRGS) $(OBJS)
	-rm *~

distclean: clean
