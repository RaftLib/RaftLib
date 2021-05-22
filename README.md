[RaftLib](http://raftlib.io) is a C++ Library for enabling stream/data-flow parallel computation. Using simple right shift operators (just like the C++ streams that you would use for string manipulation), you can link parallel compute kernels together. With RaftLib, we do away with explicit use of pthreads, std::thread, OpenMP, or any other parallel "threading" library. These are often mis-used, creating non-deterministic behavior. RaftLib's model allows lock-free FIFO-like access to the communications channels connecting each compute kernel. The full system has many auto-parallelization, optimization, and convenience features that enable relatively simple authoring of performant applications. Feel free to give it a shot, if you have any issues, please create an issue request. Minor issues, 
the Slack group is the best way to resolve. We take pull requests!! For benchmarking, feel free to send the 
authors an email. We've started a benchmark collection, however, it's far from complete. We'd love to add your 
code!! 

User Group / Mailing List: [slack channel](https://join.slack.com/t/raftlib/shared_invite/zt-3sk6ms6f-eEBd23dz98JnoRiXLaRmNw)

=============

### Build status

![CI](https://github.com/RaftLib/RaftLib/workflows/CI/badge.svg?event=push)

### Pre-requisites

#### OS X & Linux
* Compiler: c++17 capable -> Clang, GNU GCC 5.0+, or Intel icc
* Latest build runs under Linux with above compilers on both x86 and AArch64, with both pthreads and QThreads. 
* OS X M1 runs, compiles, but has some test case hiccups on templates, but doesn't seem to 
impact functionality. 
* Note for OS X users without a /user/local, specify an install prefix when using CMake. 

#### Windows
* Builds and runs under Win10

### Install
Make a build directory (for the instructions below, we'll 
write [build]). If you want to build the OpenCV example, then
you'll need to add to your cmake invocation:
```bash
-DBUILD_WOPENCV=true 
```

To use the [QThreads User space HPC threading library](http://www.cs.sandia.gov/qthreads/) 
you will need to use the version with the RaftLib org and follow the RaftLib specific readme. 
This QThreads version has patches for hwloc2.x applied and fixes for test cases. To compile
RaftLib with QThreads linked, add the following (assumes the QThreads library is in your path):
```bash
-DUSEQTHREAD=1
```

Building the examples, benchmarks and tests can be disabled using:
```bash
-DBUILD_EXAMPLES=false
-DBUILD_BENCHMARKS=false
-DBUILD_TESTS=false
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

## Using
* When building applications with RaftLib, on Linux it is best to 
use the **pkg-config** file, as an example, using the _poc.cpp_ example,
```bash
g++ `pkg-config --cflags raftlib` poc.cpp -o poc `pkg-config --libs raftlib`
```

Feel free to substitute your favorite build tool. I use Ninja and make depending on which machine I'm on. To change out, use cmake to generate the appropriate build files with the -Gxxx flag.

### Citation
If you use this framework for something that gets published, please cite it as:
```bibtex
@article{blc16,
  author = {Beard, Jonathan C and Li, Peng and Chamberlain, Roger D},
  title = {RaftLib: A C++ Template Library for High Performance Stream Parallel Processing},
  year = {2016},
  doi = {http://dx.doi.org/10.1177/1094342016672542},
  eprint = {http://hpc.sagepub.com/content/early/2016/10/18/1094342016672542.full.pdf+html},
  journal = {International Journal of High Performance Computing Applications}
}
```
### Other Info Sources
* [OpenCV wrappers for RaftLib](https://github.com/RaftLib/RaftOCV)
* [Project web page](http://raftlib.io)
* [Project wiki page](https://github.com/jonathan-beard/RaftLib/wiki)
* [Blog post intro](https://goo.gl/4VDlbr)
* [Jonathan Beard's thesis](http://goo.gl/obkWUh)
* [Views on parallel computing, general philosphy](https://goo.gl/R5fQAl)
* Feel free to e-mail one of the authors of the repo
