CC = gcc
CPP = g++

CFLAGS = -fno-pie -fno-pic -no-pie -Wall -O2 -I. -m32 -ggdb3 -fstack-check=no -fno-stack-protector -U_FORTIFY_SOURCE -D_FORTIFY_SOURCE=0 -fno-plt -U__cplusplus -D__cplusplus=201401

LDFLAGS=-Wl,-z,execstack -Wl,-z,norelro -Wl,-z,noseparate-code -Wl,-Ttext-segment,0x08040000 -Wl,-T,elf_i386.xc

OBJS1 = $(patsubst %.cc,%.o,$(wildcard *.cc))
OBJS2 = $(patsubst %.s,%.o,$(wildcard *.s))

all: perplexity

direction.door.o: direction.door.cc
	$(CPP) -c $^ $(CFLAGS) -o $@
	python alter_userbuff.py $@

%.o: %.s
	$(CC) -c $^ $(CFLAGS) -o $@

%.o: %.cc
	$(CPP) -c $^ $(CFLAGS) -o $@

perplexity: $(OBJS1) $(OBJS2)
	$(CPP) $(LDFLAGS) $(CFLAGS) -o perplexity $(OBJS1) $(OBJS2) -L../../NautilusRand -lNautilusRand32
	python fixup_elf.py perplexity
	strip -s perplexity

clean:
	rm -f *.o perplexity
