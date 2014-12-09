CC  ?= gcc
CXX ?= g++

ifneq ($(shell uname -s), Darwin)
RT = -lrt
endif

#TEST = -O0 -g -DUNITTEST=1

CFLAGS   = -O3 -mtune=native  -Wall -std=c99 $(TEST)
CXXFLAGS = -O3 -mtune=native  -Wall -std=c++11 -DRDTSCP=1  $(TEST)
LDLIBS  = -lstdc++ -lraft $(RT)

EXE = allocate portTypeException dynallocate stdlibsupport split \
      join lambdatest allocate_s foreach peekrange

OBS = $(addsuffix .o, $(EXE))

all: $(EXE)

allocate: allocate.o
portTypeException: portTypeException.o
dynallocate: dynallocate.o
stdlibsupport: stdlibsupport.o
split: split.o
join: join.o
lambdatest: lambdatest.o
allocate_s: allocate_s.o
foreach: foreach.o
peekrange: peekrange.o

.PHONY: clean
clean:
	rm -rf $(EXE) $(OBS) intelremotemonfifo*
