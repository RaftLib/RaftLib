CC  ?= gcc
CXX ?= g++

ifneq ($(shell uname -s), Darwin)
RT = -lrt
endif

CFLAGS = -O2 -Wall -std=c99
CXXFLAGS = -O2 -Wall -std=c++11 -DRDTSCP=1

CFILES   =
COBJS    =

CXXFILES = portTypeException.cpp
CXXOBJS  = portTypeException.o


INCS = 

LDFLAGS = 
LIBS = -lraftlight $(RT)

compile: $(CXXFILES) $(CFILES)
	$(MAKE) $(CXXOBJS) $(COBJS)
	$(CXX)  $(LDFLAGS) $(CXXFLAGS) -o portTypeException  portTypeException.o $(LIBS)

.PHONY: clean
clean:
	rm -rf portTypeException  $(CXXOBJS) $(COBJS)
