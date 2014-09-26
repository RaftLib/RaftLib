CC  ?= gcc
CXX ?= g++

ifneq ($(shell uname -s), Darwin)
RT = -lrt
endif

CFLAGS = -O2 -Wall -std=c99
CXXFLAGS = -O2 -Wall -std=c++11 -DRDTSCP=1

CFILES   =
COBJS    =

CXXFILES = sumapp.cpp
CXXOBJS  = sumapp.o


INCS = 

LDFLAGS = 
LIBS = -lraftlight $(RT)

compile: $(CXXFILES) $(CFILES)
	$(MAKE) $(CXXOBJS) $(COBJS)
	$(CXX)  $(LDFLAGS) $(CXXFLAGS) -o sumapp sumapp.o $(LIBS)

.PHONY: clean
clean:
	rm -rf sumapp $(CXXOBJS) $(COBJS)
