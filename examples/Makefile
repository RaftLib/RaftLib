CC  = clang
CXX = clang++
AR  = ar

ifneq ($(shell uname -s), Darwin)
LINUXLIBS 	 = -lrt -lpthread
LINUXLDFLAGS = -static -static-libgcc -static-libstdc++ 
endif

CFLAGS   = -Ofast -mtune=native -Wall -std=c99
CXXFLAGS = -Ofast -mtune=native -Wall -std=c++11 -DRDTSCP=1
LDFLAGS  = -L./ $(LINUXLDFLAGS)
LDLIBS   = -lstdc++ -lraft -lgsl  $(LINUXLIBS)

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
