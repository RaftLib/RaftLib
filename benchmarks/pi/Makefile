CC  ?= gcc
CXX ?= g++
AR  ?= ar

ifneq ($(shell uname -s), Darwin)
LINUXLIBS 	 = -lrt -lpthread
LINUXLDFLAGS = -static -static-libgcc -static-libstdc++ 
endif

CFLAGS   = -Ofast -mtune=native -Wall -std=c99
CXXFLAGS = -Ofast -mtune=native -Wall -std=c++11 -DRDTSCP=1
LDFLAGS  = -L./ $(LINUXLDFLAGS)
LDLIBS   = -lstdc++ -lraft -lgsl -lcmdargs $(LINUXLIBS)

EXE      = pisim
OBJS     = $(addsuffix .o, $(CPPFILES)) $(addsuffix .o, $(EXE))

all: $(EXE)

pisim: pisim.o

.PHONY: clean
clean:
	rm -rf $(EXE) $(OBJS) intelremotemonfifo*
