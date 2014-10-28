CC  ?= gcc
CXX ?= g++

ifneq ($(shell uname -s), Darwin)
RT = -lrt
endif

CFLAGS   = -O3 -g  -Wall -std=c99
CXXFLAGS = -O3 -g  -Wall -std=c++11 -DRDTSCP=1

CFILES   =
COBJS    =

CXXFILES = sumapp.cpp readtest.cpp singlequeue.cpp
CXXOBJS  = sumapp.o readtest.o singlequeue.o
EXENAMES = sumapp readtest singlequeue

INCS = 

LDFLAGS = 
LIBS = -lraft $(RT)

compile: $(CXXFILES) $(CFILES)
	$(MAKE) $(CXXOBJS) $(COBJS)
	$(CXX)  $(LDFLAGS) $(CXXFLAGS) -o sumapp sumapp.o $(LIBS)
	$(CXX)  $(LDFLAGS) $(CXXFLAGS) -o readtest readtest.o $(LIBS)
	$(CXX)  $(LDFLAGS) $(CXXFLAGS) -o singlequeue singlequeue.o $(LIBS)

.PHONY: clean
clean:
	rm -rf $(EXENAMES) $(CXXOBJS) $(COBJS)
