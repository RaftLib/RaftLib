RaftLib is a C++ Library for enabling stream/data-flow parallel computation. Using simple right shift operators (just like the C++ streams that you would use for string manipulation), you can link parallel compute kernels together. With RaftLib, we do away with explicit use of pthreads, std::thread, OpenMP, or any other parallel "threading" library. These are often mis-used, creating non-deterministic behavior. RaftLib's model allows lock-free FIFO-like access to the communications channels connecting each compute kernel. The full system has many auto-parallelization, optimization, and convenience features that enable relatively simple authoring of performant applications. This project is currently in the alpha stage (recently emerging from a PhD thesis). The beta release will bring back multi-node support, along with (planned) container support for the remote machines. Feel free to give it a shot, if you have any issues, also feel free to send the authors an e-mail.

=============

### Build status
[![Build Status](https://travis-ci.org/RaftLib/RaftLib.svg?branch=master)](https://travis-ci.org/RaftLib/RaftLib)

CI Test environment:
* Linux - kernel v. 3.13, Ubuntu 5.2.1-23ubuntu1~12.04, (gcc-5.2.1/Clang 3.7.1), CMake 3.5

Offline testing:
* OS X - El Capitan, Apple LLVM version 7.0.2, CMake 3.5

### Pre-requisites

####OS X & Linux
Compiler: c++14 capable -> Clang, GNU GCC 5.0+, or Intel icc
Libraries: 
* Boost, if not installed, needed headers automatically downloaded with cmake

####Windows
In progress

### Install
Make a build directory (for the instructions below, we'll 
write [build]). If you want to build the OpenCV example, then
you'll need to add to your cmake invocation:
```bash
-DBUILD_WOPENCV=true 
```
If you want to use the Scotch partitioning library (Unix/Linux) for partitioning kernel threads amongst physical resources, add
the following config line to your cmake invocation:
```bash
-DUSESCOTCH=1
```

To build:

```bash
mkdir [build]
cd [build]
cmake ..
make && make test
sudo make install
```
NOTE: The default prefix in the makefile is: 
```
PREFIX ?= /usr/local
```
Feel free to substitute your favorite build tool. I use Ninja and make depending on which machine I'm on. To change out, use cmake to generate the appropriate build files with the -Gxxx flag.

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
* [Project web page](http://raftlib.io)
* [Project wiki page](https://github.com/jonathan-beard/RaftLib/wiki)
* [Blog post intro](https://goo.gl/4VDlbr)
* [Jonathan Beard's thesis](http://goo.gl/obkWUh)
* [Views on parallel computing, general philosphy](https://goo.gl/R5fQAl)
* Feel free to e-mail one of the authors of the repo

###Random Notes
The old Makefile had an uninstall script, I need to add an object to the 
cmake file so that we can have similar functionality. Theres also a bit
of cleanup to do as I transition fully from Make to CMake.

A lot of the auto-optimization stuff has been pulled out temporarily while
I'm working on cross-platform compatibility. A lot of the low level API
calls are well, low level and Linux/Unix/OS X specific so I'm working on
building in Windows versions of those specific calls. Should be done
in a few weeks. ( 5 Jan 2016, jcb )

Added in prefetch for object types larger than a single cache line. Profiling
shows that it's really not beneficial on most platforms with good prefetch
algorithms. (25 March 2016, jcb )
