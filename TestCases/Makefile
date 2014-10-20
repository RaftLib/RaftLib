CC  ?= gcc
CXX ?= g++

ifneq ($(shell uname -s), Darwin)
RT = -lrt
endif

CFLAGS   = -O2 -Wall -std=c99
CXXFLAGS = -O2 -Wall -std=c++11 -DRDTSCP=1

EXE = allocate portTypeException
OBS = $(addsuffix .o, $(EXE))

all: $(EXE)

allocate: allocate.o

portTypeException: portTypeException.o

.PHONY: clean
clean:
	rm -rf $(EXE) $(OBS)
