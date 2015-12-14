CC  ?= clang
CXX ?= clang++
AR  ?= ar

ifneq ($(shell uname -s), Darwin)
STATIC = -static -static-libgcc -static-libstdc++
RT =  -lrt -Wl,--whole-archive -lpthread -Wl,--no-whole-archive
endif

TEST = -O0 -g3 -DUNITTEST=1 -ggdb -DDEBUG=1
RELEASE = -Ofast -mtune=native

ifeq ($(type), test)
BUILD = $(TEST)
else
BUILD = $(RELEASE)
endif

EXTRA = -I../../boost-dep/core/include/ \
        -I../../boost-dep/config/include/


CFLAGS   =  $(BUILD) -Wall -std=c99 
CXXFLAGS =  $(BUILD) $(EXTRA) `pkg-config --cflags opencv` -Wall -std=c++11 -DRDTSCP=1 -I../
LDLIBS  = $(STATIC) `pkg-config --libs opencv`  -lstdc++ -lraft -lgsl -lgslcblas $(RT) -lm 

EXE = contours

OBS = $(addsuffix .o, $(EXE))


all: $(EXE)

contours: contours.o

.PHONY: clean
clean:
	rm -rf $(EXE) $(OBS) intelremotemonfifo* *.dwarf lldb
