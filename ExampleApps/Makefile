CC  ?= gcc
CXX ?= g++

ifneq ($(shell uname -s), Darwin)
LINUXLIBS 	 = -lrt -lpthread
LINUXLDFLAGS = 
endif

CPPFLAGS = -I./ahocorasick/
CFLAGS   = -O3 -msse4.1 -mtune=native -fast -Wall -std=c99
CXXFLAGS = -O3 -msse4.1 -mtune=native -fast -Wall -std=c++11 -DRDTSCP=1
LDFLAGS  = -L./ $(LINUXLDFLAGS)
LDLIBS   = -lahocorasick -lraft $(LINUXLIBS)

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
