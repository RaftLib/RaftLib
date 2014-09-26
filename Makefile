CC    ?= gcc
CXX   ?= clang++

include fifo/buffer.makefile

DIRINCS = $(RINGBUFFERDIR)

ifneq ($(shell uname -s), Darwin)
RT = -lrt
STATIC = -static -static-libgcc -static-libstdc++
PTHREAD = -lpthread  
endif

CFLAGS   =  -O3  -Wall -std=c99
CXXFLAGS =  -O3  -Wall -std=c++11  -DRDTSCP=1 -DLIMITRATE=1


RAFTLIGHTCXXOBJS = allocate map graphtools port portexception schedule \
                   simpleschedule stdalloc

COBJS   = $(RBCOBJS)
CXXOBJS = $(RBCXXOBJS) $(RAFTLIGHTCXXOBJS)

CFILES = $(addsuffix .c, $(COBJS) )
CXXFILES = $(addsuffix .cpp, $(CXXOBJS) )

OBJS = $(addsuffix .o, $(COBJS) ) $(addsuffix .o, $(CXXOBJS) )


INCS = $(addprefix -I, $(DIRINCS))
LIBS = $(RT) $(PTHREAD)

compile: $(CXXFILES) $(CFILES)
	$(MAKE) $(OBJS)
	$(AR) rvs libraftlight.a $(OBJS) $(LIBS)

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCS) -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) $(INCS) -o $@ $<

.PHONY: clean
clean:
	rm -rf libraftlight.a $(OBJS)
