CC  ?= clang
CXX ?= clang++
AR  ?= ar

ifneq ($(shell uname -s), Darwin)
RT =  -lrt -Wl,--whole-archive -lpthread -Wl,--no-whole-archive
ALLLIBS 	 = $(RT)  `pkg-config --libs gsl`
LINUXLDFLAGS = -static #-static-libgcc -static-libstdc++ 
else
ALLLIBS = -lgsl -lgslcblas -lm
endif

CFLAGS   = -O3 -mtune=native  -Wall -std=c99
CXXFLAGS = -O3 -mtune=native  -Wall -std=c++11 -DRDTSCP=1

LDFLAGS  = -L./ $(LINUXLDFLAGS)
LDLIBS   = -lstdc++ -lraft  -lcmdargs $(ALLLIBS)

EXE      = pisim
OBJS     = $(addsuffix .o, $(CPPFILES)) $(addsuffix .o, $(EXE))


all: $(EXE)

pisim: pisim.o


.PHONY: clean
clean:
	rm -rf $(EXE) $(OBJS) intelremotemonfifo*


