CXXFLAGS=$(shell pkg-config --cflags libbitcoin) -g -Iinclude/
LIBS=$(shell pkg-config --libs libbitcoin)

default: cbind test

obj/bitcoin.o: src/bitcoin.cpp
	$(CXX) -o obj/bitcoin.o -c $< $(CXXFLAGS)

cbind: obj/bitcoin.o

obj/test.o: test.c
	$(CC) -o obj/test.o -c $< $(CXXFLAGS)

test: obj/test.o
	$(CC) -o test obj/test.o obj/bitcoin.o -lstdc++ $(LIBS)

