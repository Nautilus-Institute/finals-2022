CC = gcc

CFLAGS = -fPIC -Wall -O2 -I. -m64
LDFLAGS=-fPIC

CFLAGS32 = -fPIC -Wall -O2 -I. -m32 -D_FILE_OFFSET_BITS=64

OBJS_TEST = $(patsubst %.c,%.o,$(wildcard *.c))
OBJS_TEST32 = $(patsubst %.c,%32.o,$(wildcard *.c))
OBJS = $(filter-out test.o,$(OBJS_TEST))
OBJS32 = $(filter-out test.o,$(OBJS_TEST32))

all: libNautilusRand.so libNautilusRand32.so

%32.o: %.c
	$(CC) -c $^ $(CFLAGS32) -o $@

%.o: %.c
	$(CC) -c $^ $(CFLAGS) -o $@

libNautilusRand.so: $(OBJS)
	$(CC) $(LDFLAGS) $(CFLAGS) $(OBJS) -shared -o libNautilusRand.so
	strip -s libNautilusRand.so

libNautilusRand32.so: $(OBJS32)
	$(CC) $(LDFLAGS) $(CFLAGS32) $(OBJS32) -shared -o libNautilusRand32.so
	strip -s libNautilusRand32.so

test: libNautilusRand.so test.o
	$(CC) -c test.c $(CFLAGS) -pie -o test.o
	$(CC) $(LDFLAGS) $(CFLAGS) -o test test.o -L. -lNautilusRand

test32: libNautilusRand32.so test32.o
	$(CC) -c test.c $(CFLAGS32) -pie -o test32.o
	$(CC) $(LDFLAGS) $(CFLAGS32) -o test32 test32.o -L. -lNautilusRand32

clean:
	rm -f *.o libNautilusRand.so test
