CC    ?= gcc
CXX   ?= g++


CFLAGS   =  -O3 -g  -Wall -std=c99
CXXFLAGS =  -O3 -g  -Wall -std=c++11  -DRDTSCP=1 -DLIMITRATE=1

COBJS = getrandom
CXXOBJS = main pointer shm Clock procwait resolution fifo

CFILES = $(addsuffix .c, $(COBJS) )
CXXFILES = $(addsuffix .cpp, $(CXXOBJS) )

OBJS = $(addsuffix .o, $(COBJS) ) $(addsuffix .o, $(CXXOBJS) )

ifneq ($(shell uname -s), Darwin)
RT = -lrt
STATIC = -static -static-libgcc -static-libstdc++
endif

INCS = 
LIBS = -lpthread -lgsl -lgslcblas -lm -lc $(RT) $(STATIC) 

compile: $(CXXFILES) $(CFILES)
	$(MAKE) $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCS) $(OBJS) $(LIBS) -o ringb

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCS) -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) $(INCS) -o $@ $<

.PHONY: clean
clean:
	rm -rf ringb $(OBJS)
