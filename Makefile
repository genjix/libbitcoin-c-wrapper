CXXFLAGS=$(shell pkg-config --cflags libbitcoin) -g -Iinclude/
LIBS=$(shell pkg-config --libs libbitcoin)

default: cbind fullnode

obj/bitcoin.o: src/bitcoin.cpp
	$(CXX) -o obj/bitcoin.o -c $< $(CXXFLAGS)

cbind: obj/bitcoin.o

obj/fullnode.o: examples/fullnode.c
	$(CC) -o obj/fullnode.o -c $< $(CXXFLAGS)

fullnode: obj/fullnode.o
	$(CC) -o fullnode obj/fullnode.o obj/bitcoin.o -lstdc++ $(LIBS)

