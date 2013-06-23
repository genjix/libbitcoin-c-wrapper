CXXFLAGS=-fPIC $(shell pkg-config --cflags libbitcoin) -g -Iinclude/
LIBS=$(shell pkg-config --libs libbitcoin)

LIBNAME=libcbitcoin.so
BASE_MODULES= \
    primitives.o \
    network.o \
    blockchain.o \
    util.o \
    crypto.o \
    core.o
MODULES=$(addprefix obj/, $(BASE_MODULES))

default: cbitcoin fullnode initchain determ

obj/primitives.o: src/primitives.cpp
	$(CXX) -o obj/primitives.o -c $< $(CXXFLAGS)

obj/network.o: src/network.cpp
	$(CXX) -o obj/network.o -c $< $(CXXFLAGS)

obj/crypto.o: src/crypto.cpp
	$(CXX) -o obj/crypto.o -c $< $(CXXFLAGS)

obj/util.o: src/util.cpp
	$(CXX) -o obj/util.o -c $< $(CXXFLAGS)

obj/blockchain.o: src/blockchain.cpp
	$(CXX) -o obj/blockchain.o -c $< $(CXXFLAGS)

obj/core.o: src/core.cpp
	$(CXX) -o obj/core.o -c $< $(CXXFLAGS)

$(LIBNAME): $(MODULES)
	$(CC) -shared -o $(LIBNAME) $(MODULES)

cbitcoin: $(LIBNAME)

obj/determ.o: examples/determ.c
	$(CC) -o obj/determ.o -c $< $(CXXFLAGS)

determ: obj/determ.o cbitcoin
	$(CC) -o determ obj/determ.o -L. -lcbitcoin -lstdc++ $(LIBS)

obj/initchain.o: examples/initchain.c
	$(CC) -o obj/initchain.o -c $< $(CXXFLAGS)

initchain: obj/initchain.o cbitcoin
	$(CC) -o initchain obj/initchain.o -L. -lcbitcoin -lstdc++ $(LIBS)

obj/fullnode.o: examples/fullnode.c
	$(CC) -o obj/fullnode.o -c $< $(CXXFLAGS)

fullnode: obj/fullnode.o cbitcoin
	$(CC) -o fullnode obj/fullnode.o -L. -lcbitcoin -lstdc++ $(LIBS)

