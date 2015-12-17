CC  ?= clang
CXX ?= clang++
AR  ?= ar

ifneq ($(shell uname -s), Darwin)
LINUXLIBS 	 = -lrt -lpthread #-Wl,--whole-archive -lpthread -Wl,--no-whole-archive
#LINUXLDFLAGS = -static -static-libgcc -static-libstdc++ 
endif

TEST = -O0 -g -DUNITTEST=1 -ggdb -DDEBUG=1
RELEASE = -Ofast -mtune=native 

ifeq ($(type), test)
BUILD = $(TEST)
else
BUILD = $(RELEASE)
endif

CFLAGS   = $(BUILD) -Wall -std=c99
CXXFLAGS = $(BUILD) -Wall -std=c++11 -DRDTSCP=1 \
           -I../../boost-dep/core/include \
           -I../../boost-dep/config/include

LDFLAGS  = -L./ $(LINUXLDFLAGS)
LDLIBS   = -lstdc++ -lraft -lgsl -lgslcblas -lm $(LINUXLIBS)

EXE      = sumapp readtest singlequeue multapp
OBJS     = $(addsuffix .o, $(CPPFILES)) $(addsuffix .o, $(EXE))

all: $(EXE)

sumapp: sumapp.o
multapp: multapp.o
readtest: readtest.o
singlequeue: singlequeue.o

.PHONY: clean
clean:
	rm -rf $(EXE) $(OBJS) intelremotemonfifo*
