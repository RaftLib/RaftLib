CC  ?= clang
CXX ?= clang++
AR  ?= ar

ifneq ($(shell uname -s), Darwin)
LINUXLIBS 	 = -lrt -Wl,--whole-archive -lpthread -Wl,--no-whole-archive
LINUXLDFLAGS = -static #-static-libgcc -static-libstdc++ 
endif

CFLAGS   = -O3 -mtune=native  -Wall -std=c99
CXXFLAGS = -I./ahocorasick/ -I../../fifo/ -O3 -mtune=native -Wall -std=c++11 -DRDTSCP=1
LDFLAGS  = -L./ $(LINUXLDFLAGS)
LDLIBS   = -lahocorasick -lstdc++ -lraft -lgsl -lgslcblas -lm $(LINUXLIBS)

EXE      = rgrep enum_rk_rgrep enum_rgrep 
OBJS     = $(addsuffix .o, $(CPPFILES)) $(addsuffix .o, $(EXE))

all: $(EXE)

rgrep: rgrep.o
enum_rk_rgrep: enum_rk_rgrep.o
enum_rgrep: enum_rgrep.o

.PHONY: clean
clean:
	rm -rf $(EXE) $(OBJS) intelremotemonfifo*
