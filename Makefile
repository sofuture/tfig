COPTS += -std=c99 -Wall -pedantic
LIBS += -lm

OBJS = ll.o

tfig: tfig.c ${OBJS}
	$(CC) -o $@ $< $(COPTS) $(OBJS) $(LIBS)

clean:
	rm -rf tfig *.o

