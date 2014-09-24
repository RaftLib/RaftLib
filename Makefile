CC    ?= gcc
CXX   ?= g++

include fifo/buffer.makefile

DIRINCS = $(RINGBUFFERDIR)

ifneq ($(shell uname -s), Darwin)
RT = -lrt
STATIC = -static -static-libgcc -static-libstdc++
endif

CFLAGS   =  -O2 -g  -Wall -std=c99
CXXFLAGS =  -O2 -g  -Wall -std=c++11  -DRDTSCP=1 -DLIMITRATE=1


RAFTLIGHTCXXOBJS = allocate map graphtools port portexception schedule \
                   simpleschedule stdalloc

COBJS   = $(RBCOBJS)
CXXOBJS = $(RBCXXOBJS) $(RAFTLIGHTCXXOBJS)

CFILES = $(addsuffix .c, $(COBJS) )
CXXFILES = $(addsuffix .cpp, $(CXXOBJS) )

OBJS = $(addsuffix .o, $(COBJS) ) $(addsuffix .o, $(CXXOBJS) )


INCS = $(addprefix -I, $(DIRINCS))
LIBS = -lpthread  $(RT)

compile: $(CXXFILES) $(CFILES)
	$(MAKE) $(OBJS)
	$(AR) rcs raftlight.a $(OBJS)
	#$(CXX) -dynamiclib -o raftlight.dylib $(OBJS)
%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCS) -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) $(INCS) -o $@ $<

.PHONY: clean
clean:
	rm -rf raftlight.a $(OBJS)
