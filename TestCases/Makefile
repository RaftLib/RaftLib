CC  ?= gcc
CXX ?= g++

ifneq ($(shell uname -s), Darwin)
RT = -lrt
endif

CFLAGS   = -O0 -g -Wall -std=c99
CXXFLAGS = -O0 -g  -Wall -std=c++11 -DRDTSCP=1 -DUNITTEST=1
LDFLAGS  = -lraftlight

EXE = allocate portTypeException dynallocate stdlibsupport 
OBS = $(addsuffix .o, $(EXE))

all: $(EXE)

allocate: allocate.o
portTypeException: portTypeException.o
dynallocate: dynallocate.o
stdlibsupport: stdlibsupport.o

.PHONY: clean
clean:
	rm -rf $(EXE) $(OBS) intelremotemonfifo*
