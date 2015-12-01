

=============

### Build status
[![Build Status](https://travis-ci.org/jonathan-beard/RaftLib.svg?branch=master)](https://travis-ci.org/jonathan-beard/RaftLib)

Test environment:
* Linux - kernel v. 3.13, Ubuntu 5.2.1-23ubuntu1~12.04, (gcc-5.2.1/Clang 3.7), CMake 3.4
* OS X - El Capitan, Apple LLVM version 7.0.0, CMake 3.4

### Pre-requisites

####OS X
Compiler: Clang, GNU GCC 4.8+, or Intel icc
Libraries: 
* Only for random number generators <a href="http://goo.gl/gchdSw">GNU GSL</a> 
* Only for partitioning, basic download doesn't need: <a href="http://goo.gl/tI1NGf">Scotch</a>

####Linux
Compiler: c++11 capable 
Libraries: 
* Only for random number generators <a href="http://goo.gl/gchdSw">GNU GSL</a> 
* Only for partitioning, basic download doesn't need: <a href="http://goo.gl/tI1NGf">Scotch</a>

####Windows
In progress

### Install
Once the optional pre-requisite libraries are installed,
make a build directory (for the instructions below, we'll 
write [build]).
```bash
mkdir [build]
cd [build]
cmake ..
make && make test
sudo make install
```
NOTE: The default prefix in the makefile is 
```
PREFIX ?= /usr/local
```
The old Makefile had an uninstall script, I need to add an object to the 
cmake file so that we can have similar functionality. 

NOTE: still working on cmake/re-arrangeing. CMake basically works
tested on OS X and Linux. I'll have more time for re-arrangements and
the sub-modules sometime this week (19 Nov. 2015)
