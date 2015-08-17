CC    ?= gcc
CXX   ?= g++
AR    ?= ar

PREFIX ?= /usr/local

include fifo/buffer.makefile
include packages/packages.makefile
#include libfiber/libfiber.makefile

DIRINCS = $(PACKAGEDIR) $(RINGBUFFERDIR) ./

ifneq ($(shell uname -s), Darwin)
RT = -lrt
STATIC = -DRDTSCP=1 -static -static-libgcc -static-libstdc++
PTHREAD = -lpthread  
endif

type =? release
TEST = -O0 -g 
RELEASE = -Ofast -mtune=native

ifeq ($(type), test)
BUILD = $(TEST) $(STATIC) -DDEBUG=1
else
BUILD = $(RELEASE) $(STATIC)
endif


CFLAGS   =  $(BUILD) -Wall -std=c99 
CXXFLAGS =  $(BUILD) -Wall -std=c++11 #-Wstack-protector -fstack-protector-all


RAFTLIGHTCXXOBJS = allocate map graphtools port portexception schedule \
                   simpleschedule stdalloc portiterator dynalloc \
                   roundrobin kernel mapbase submap globalmap \
                   systemsignalhandler poolschedule kernelcontainer \
                   common basicparallel noparallel affinity

COBJS   = $(RBCOBJS) $(LIBFIBERCOBJS) pthreadwrap
CXXOBJS = $(PACKAGEOBJS) $(RBCXXOBJS) $(RAFTLIGHTCXXOBJS)

CFILES = $(addsuffix .c, $(COBJS) )
CXXFILES = $(addsuffix .cpp, $(CXXOBJS) )

OBJS = $(addsuffix .o, $(COBJS) ) $(addsuffix .o, $(CXXOBJS) )


INCS = $(addprefix -I, $(DIRINCS))
LIBS = $(RT) $(PTHREAD) $(PACKAGELIBS) 

compile: $(CXXFILES) $(CFILES)
	$(MAKE) $(OBJS)
	$(AR) rvs libraft.a $(OBJS)

install:
	cp libraft.a $(PREFIX)/lib/
	mkdir -p $(PREFIX)/include/raft_dir
	cp *.hpp $(PREFIX)/include/raft_dir/
	cp *.h   $(PREFIX)/include/raft_dir/
	cp *.tcc $(PREFIX)/include/raft_dir/
	cp ./fifo/*.hpp $(PREFIX)/include/raft_dir/
	cp ./fifo/*.tcc $(PREFIX)/include/raft_dir/
	cp ./packages/* $(PREFIX)/include/raft_dir/
	cp raft $(PREFIX)/include/
	cp raftio $(PREFIX)/include/
	cp raftrandom $(PREFIX)/include/
	cp raftstat $(PREFIX)/include/
	cp raftutility $(PREFIX)/include/
	cp raftmath $(PREFIX)/include/
	echo "Install complete!"

uninstall:
	rm -rf $(PREFIX)/lib/libraft.a
	rm -rf $(PREFIX)/include/raft_dir
	rm -rf $(PREFIX)/include/raft
	rm -rf $(PREFIX)/include/raftio
	rm -rf $(PREFIX)/include/raftrandom
	rm -rf $(PREFIX)/include/raftstat
	rm -rf $(PREFIX)/include/raftutility
	rm -rf $(PREFIX)/include/raftmath
	echo "Uninstalled!"

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCS) -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) $(INCS) -o $@ $<

.PHONY: clean
clean:
	rm -rf libraft.a $(OBJS) intelremotemonfifo.*
