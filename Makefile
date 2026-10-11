CFLAGS += -std=c99 -Wall -Wextra -pedantic
LIBS += -lm

OBJS = ll.o

.PHONY: all test clean

all: tfig tfig_debug

tfig: tfig.c ll.h ${OBJS}
	$(CC) -o $@ tfig.c $(CFLAGS) $(OBJS) $(LIBS)

tfig_debug: tfig.c ll.h ${OBJS}
	$(CC) -o $@ tfig.c $(CFLAGS) $(OBJS) $(LIBS) -DDEBUG

ll.o: ll.c ll.h
	$(CC) $(CFLAGS) -c -o $@ ll.c

test: tfig
	./run_tests

clean:
	rm -rf tfig tfig_debug *.o
