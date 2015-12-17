CC  ?= clang
CXX ?= clang++
AR  ?= ar

ifneq ($(shell uname -s), Darwin)
LINUXLIBS 	 = -lrt -lpthread #-Wl,--whole-archive -lpthread -Wl,--no-whole-archive
#LINUXLDFLAGS = -static -static-libgcc -static-libstdc++ 
endif

TEST = -O0 -g3 -DUNITTEST=1 -ggdb -DDEBUG=1
RELEASE = -Ofast -mtune=native 

ifeq ($(type), test)
BUILD = $(TEST)
else
BUILD = $(RELEASE)
endif

EXTRA = -I../../../boost-dep/core/include/ \
        -I../../../boost-dep/config/include/

CFLAGS   = $(BUILD) ${EXTRA} -Wall -std=c99
CXXFLAGS = $(BUILD) $(INC) ${EXTRA} -Wall -std=c++11 -DRDTSCP=1
LDFLAGS  = -L./ $(LINUXLDFLAGS)
LDLIBS   = -lstdc++ -lraft -lm $(LINUXLIBS)

EXE      = bmh  
OBJS     = $(addsuffix .o, $(CPPFILES)) $(addsuffix .o, $(EXE))

all: $(EXE)

bmh: bmh.o

.PHONY: clean
clean:
	rm -rf $(EXE) $(OBJS) intelremotemonfifo*
