CC    ?= gcc
CXX   ?= g++

PREFIX = /usr/local

include fifo/buffer.makefile

DIRINCS = $(RINGBUFFERDIR)

ifneq ($(shell uname -s), Darwin)
RT = -lrt
STATIC = -static -static-libgcc -static-libstdc++
PTHREAD = -lpthread  
endif

CFLAGS   =  -O1 -g  -Wall -std=c99
CXXFLAGS =  -O1 -g  -Wall -std=c++11  -DRDTSCP=1 -DLIMITRATE=1


RAFTLIGHTCXXOBJS = allocate map graphtools port portexception schedule \
                   simpleschedule stdalloc portiterator dynalloc

COBJS   = $(RBCOBJS)
CXXOBJS = $(RBCXXOBJS) $(RAFTLIGHTCXXOBJS)

CFILES = $(addsuffix .c, $(COBJS) )
CXXFILES = $(addsuffix .cpp, $(CXXOBJS) )

OBJS = $(addsuffix .o, $(COBJS) ) $(addsuffix .o, $(CXXOBJS) )


INCS = $(addprefix -I, $(DIRINCS))
LIBS = $(RT) $(PTHREAD)

compile: $(CXXFILES) $(CFILES)
	$(MAKE) $(OBJS)
	$(AR) rvs libraftlight.a $(OBJS)

install:
	cp libraftlight.a $(PREFIX)/lib/
	mkdir -p $(PREFIX)/include/raft_dir
	cp *.hpp $(PREFIX)/include/raft_dir/
	cp ./fifo/*.hpp $(PREFIX)/include/raft_dir/
	cp ./fifo/*.tcc $(PREFIX)/include/raft_dir/
	cp raft $(PREFIX)/include/
	echo "Install complete!"

uninstall:
	rm -rf $(PREFIX)/lib/libraftlight.a
	rm -rf $(PREFIX)/include/raft_dir
	rm -rf $(PREFIX)/include/raft
	echo "Uninstalled!"
%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) $(INCS) -o $@ $<

%.o: %.c
	$(CC) -c $(CFLAGS) $(INCS) -o $@ $<

.PHONY: clean
clean:
	rm -rf libraftlight.a $(OBJS)
