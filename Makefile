CXXFLAGS=$(shell pkg-config --cflags libbitcoin) -g -Iinclude/
LIBS=$(shell pkg-config --libs libbitcoin)

default: cbind fullnode initchain

obj/bitcoin.o: src/bitcoin.cpp
	$(CXX) -o obj/bitcoin.o -c $< $(CXXFLAGS)

cbind: obj/bitcoin.o

obj/initchain.o: examples/initchain.c
	$(CC) -o obj/initchain.o -c $< $(CXXFLAGS)

initchain: obj/initchain.o obj/bitcoin.o
	$(CC) -o initchain obj/initchain.o obj/bitcoin.o -lstdc++ $(LIBS)

obj/fullnode.o: examples/fullnode.c
	$(CC) -o obj/fullnode.o -c $< $(CXXFLAGS)

fullnode: obj/fullnode.o obj/bitcoin.o
	$(CC) -o fullnode obj/fullnode.o obj/bitcoin.o -lstdc++ $(LIBS)

