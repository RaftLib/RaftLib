CC  ?= gcc
CXX ?= g++

ifneq ($(shell uname -s), Darwin)
LINUXLIBS 	 = -lrt -lpthread
LINUXLDFLAGS = -static -static-libgcc -static-libstdc++
endif

CFLAGS   = -O3 -msse4.1 -Wall -std=c99
CXXFLAGS = -O3 -msse4.1 -Wall -std=c++11 -DRDTSCP=1
LDFLAGS  = $(LINUXLDFLAGS)
LDLIBS   = -lraft $(LINUXLIBS)

EXE      = sumapp readtest singlequeue rgrep
CPPFILES = rkverifymatch
OBJS     = $(addsuffix .o, $(CPPFILES)) $(addsuffix .o, $(EXE))

all: $(EXE)

sumapp: sumapp.o
readtest: readtest.o
singlequeue: singlequeue.o
rgrep: rgrep.o rkverifymatch.o
	$(CXX) $(CXXFLAGS) $(LDFLAGS)  $^ -o $@ $(LDLIBS)
rkverifymatch.o: rkverifymatch.cpp

.PHONY: clean
clean:
	rm -rf $(EXE) $(OBJS) intelremotemonfifo*
