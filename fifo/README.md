simpleringbuffer
================

A simple ringbuffer template implementation in C++ that
can utilize local or shared memory.  

**USE**
Compile with -std=c++11 flag, there's an example threaded
app in main.cpp.  One limitation of the SHM version is that
the buffer must be statically sized.  The locally allocated
version can be allocated on the fly.  

**TODO**
* Add TCP connected ringbuffer implementation.
* Add Java implementation that can use the C/C++ allocated SHM with at least primitive types.
* Add write and read optimizations.
