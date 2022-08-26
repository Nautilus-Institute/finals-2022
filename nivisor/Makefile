INCLUDE ?= inc

BIN=nivisor
SRC=$(wildcard src/*.cpp)
OBJS=$(notdir $(patsubst %.cpp, %.o, $(SRC)))

all: $(OBJS) stub.o sha1.o
	g++ -o $(BIN) $^

clean:
	-rm *.o

sha1.o: src/sha1.c
	gcc -c $^ -I $(INCLUDE)

stub.o: src/stub.S
	gcc -c $^

$(OBJS): %.o : src/%.cpp
	g++ $(CFLAGS) $(CPPFLAGS) -c $< -I $(INCLUDE)

tests:
	make -C test_files/
