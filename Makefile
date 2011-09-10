CFLAGS += -std=c99 -Wall -pedantic
LIBS += -lm

OBJS = ll.o

.PHONY: all
all: tfig tfig_debug

tfig: tfig.c ${OBJS}
	$(CC) -o $@ $< $(CFLAGS) $(OBJS) $(LIBS)

tfig_debug: tfig.c ${OBJS}
	$(CC) -o $@ $< $(CFLAGS) $(OBJS) $(LIBS) -DDEBUG

clean:
	rm -rf tfig *.o

