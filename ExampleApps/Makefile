CC  ?= gcc
CXX ?= g++

ifneq ($(shell uname -s), Darwin)
RT = -lrt -static -static-libgcc -static-libstdc++ -lpthread
endif

CFLAGS   = -O3   -Wall -std=c99
CXXFLAGS = -O3   -Wall -std=c++11 -DRDTSCP=1

CFILES   =
COBJS    =

CXXFILES = sumapp.cpp readtest.cpp singlequeue.cpp rgrep.cpp rkverifymatch.cpp
CXXOBJS  = sumapp.o readtest.o singlequeue.o rgrep.o rkverifymatch.o
EXENAMES = sumapp readtest singlequeue rgrep

INCS = 

LDFLAGS = 
LIBS = -lraft $(RT)

compile: $(CXXFILES) $(CFILES)
	$(MAKE) $(CXXOBJS) $(COBJS)
	$(CXX)  $(LDFLAGS) $(CXXFLAGS) -o sumapp sumapp.o $(LIBS)
	$(CXX)  $(LDFLAGS) $(CXXFLAGS) -o readtest readtest.o $(LIBS)
	$(CXX)  $(LDFLAGS) $(CXXFLAGS) -o singlequeue singlequeue.o $(LIBS)
	$(CXX)  $(LDFLAGS) $(CXXFLAGS) -o rgrep rgrep.o rkverifymatch.o $(LIBS)
.PHONY: clean
clean:
	rm -rf $(EXENAMES) $(CXXOBJS) $(COBJS)
