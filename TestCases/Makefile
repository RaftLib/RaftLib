CC  = gcc
CXX = g++
AR  = ar

ifneq ($(shell uname -s), Darwin)
STATIC = -static -static-libgcc -static-libstdc++
RT =  -lrt -Wl,--whole-archive -lpthread -Wl,--no-whole-archive
endif

TEST = -O0 -g -DUNITTEST=1 -ggdb
RELEASE = -Ofast -mtune=native 

ifeq ($(type), test)
BUILD = $(TEST)
else
BUILD = $(RELEASE)
endif

CFLAGS   =  $(BUILD) -Wall -std=c99 
CXXFLAGS =  $(BUILD) -Wall -std=c++11 -DRDTSCP=1 
LDLIBS  = $(STATIC)  -lstdc++ -lraft -lgsl $(RT) -lm 

EXE = allocate portTypeException dynallocate stdlibsupport split \
      join lambdatest allocate_s foreach peekrange gamma uniform \
      gaussian exponential partitionTest iterator sequential

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
gamma: gamma.o
uniform: uniform.o
gaussian: gaussian.o
exponential: exponential.o
partitionTest: partitionTest.o
iterator: iterator.o
sequential: sequential.o

.PHONY: clean
clean:
	rm -rf $(EXE) $(OBS) intelremotemonfifo* *.dwarf
