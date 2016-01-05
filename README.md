

=============

### Build status
[![Build Status](https://travis-ci.org/jonathan-beard/RaftLib.svg?branch=master)](https://travis-ci.org/jonathan-beard/RaftLib)

CI Test environment:
* Linux - kernel v. 3.13, Ubuntu 5.2.1-23ubuntu1~12.04, (gcc-5.2.1/Clang 3.7), CMake 3.4

Offline testing:
* OS X - El Capitan, Apple LLVM version 7.0.0, CMake 3.4

### Pre-requisites

####OS X & Linux
Compiler: c++11 capable -> Clang, GNU GCC 4.8+, or Intel icc
Libraries: 
* Boost, if not installed, needed hearders automatically downloaded with cmake


####Windows
In progress

### Install
Make a build directory (for the instructions below, we'll 
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
###Citation
If you use this framework for something that gets published, please cite it as:
```bibtex
@inproceedings{blc15,
  author = {Beard, Jonathan C. and Li, Peng and Chamberlain, Roger D.},
  title = {RaftLib: A {C++} Template Library for High Performance Stream Parallel Processing},
  publisher = {ACM},
  address = {New York, NY, USA},
  year = {2015},
  month = feb,
  series = {PMAM 2015},
  booktitle = {Proceedings of Programming Models and Applications on Multicores and Manycores},
  pages = {96-105},
}
```
###Other Info Sources
* Project web page -> http://raftlib.io
* Blog post intro -> https://goo.gl/4VDlbr
* Jonathan Beard's thesis -> http://goo.gl/obkWUh
* Feel free to e-mail one of the authors of the repo

###Random Notes
The old Makefile had an uninstall script, I need to add an object to the 
cmake file so that we can have similar functionality. 

A lot of the auto-optimization stuff has been pulled out temporaril while
I'm working on cross-platform compatibility. A lot of the low level API
calls are well, low level and Linux/Unix/OS X specific so I'm working on
building in Windows versions of those specific calls. Should be done
in a few weeks. ( 5 Jan 2015, jcb )
