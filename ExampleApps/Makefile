CC  ?= gcc
CXX ?= g++

ifneq ($(shell uname -s), Darwin)
RT = -lrt -static -static-libgcc -static-libstdc++ -lpthread
endif

CFLAGS   = -O3 -fast -msse4.1 -Wall -std=c99
CXXFLAGS = -O3 -fast -msse4.1 -Wall -std=c++11 -DRDTSCP=1
LDLIBS   = -lraft $(RT)

EXE      = sumapp readtest singlequeue rgrep
CPPFILES = rkverifymatch
OBJS     = $(addsuffix .o, $(CPPFILES)) $(addsuffix .o, $(EXE))

all: $(EXE)

sumapp: sumapp.o
readtest: readtest.o
singlequeue: singlequeue.o
rgrep: rgrep.o rkverifymatch.o
rkverifymatch.o: rkverifymatch.cpp

.PHONY: clean
clean:
	rm -rf $(EXE) $(OBJS) intelremotemonfifo*
