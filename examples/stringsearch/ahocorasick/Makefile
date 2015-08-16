CC ?= icc
LIBNAME := libahocorasick.a
CFLAGS := -Ofast -mtune=native -Wall
HEADERS := actypes.h ahocorasick.h node.h
BUILD_DIRECTORY := ../

all: $(LIBNAME)
 
$(LIBNAME): ahocorasick.o node.o
	ar -cvq $(BUILD_DIRECTORY)/$(LIBNAME) $(BUILD_DIRECTORY)/ahocorasick.o $(BUILD_DIRECTORY)/node.o

ahocorasick.o: ahocorasick.c $(HEADERS)
	$(CC) -o $(BUILD_DIRECTORY)/ahocorasick.o -c ahocorasick.c $(CFLAGS)

node.o: node.c $(HEADERS)
	$(CC) -o $(BUILD_DIRECTORY)/node.o -c node.c $(CFLAGS)


clean:
	rm -rf $(BUILD_DIRECTORY)/$(LIBNAME) $(BUILD_DIRECTORY)/ahocorasick.o \
	$(BUILD_DIRECTORY)/node.o
