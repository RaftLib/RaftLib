CC  ?= gcc
CXX ?= g++

ifneq ($(shell uname -s), Darwin)
RT = -lrt
endif

CFLAGS = -O0 -g -Wall -std=c99
CXXFLAGS = -O1   -Wall -std=c++11 -DRDTSCP=1

CFILES   =
COBJS    =

CXXFILES = sumapp.cpp readtest.cpp
CXXOBJS  = sumapp.o readtest.o


INCS = 

LDFLAGS = 
LIBS = -lraftlight $(RT)

compile: $(CXXFILES) $(CFILES)
	$(MAKE) $(CXXOBJS) $(COBJS)
	$(CXX)  $(LDFLAGS) $(CXXFLAGS) -o sumapp sumapp.o $(LIBS)
	$(CXX)  $(LDFLAGS) $(CXXFLAGS) -o readtest readtest.o $(LIBS)

.PHONY: clean
clean:
	rm -rf readtest sumapp $(CXXOBJS) $(COBJS)
