[RaftLib](http://raftlib.io) is a C++ Library for enabling stream/data-flow parallel computation. Using simple right shift operators (just like the C++ streams that you would use for string manipulation), you can link parallel compute kernels together. With RaftLib, we do away with explicit use of pthreads, std::thread, OpenMP, or any other parallel "threading" library. These are often mis-used, creating non-deterministic behavior. RaftLib's model allows lock-free FIFO-like access to the communications channels connecting each compute kernel. The full system has many auto-parallelization, optimization, and convenience features that enable relatively simple authoring of performant applications. This project is currently in the alpha stage. The beta release will bring back multi-node support, along with (planned) container support for the remote machines. Feel free to give it a shot, if you have any issues, also feel free to send the authors an e-mail.

User Group / Mailing List: [slack channel](https://join.slack.com/t/raftlib/shared_invite/enQtMjk3MjYyODYxODYzLWM5YmMxZTE1YmY0NDIwZTFkNmE1OThhMTUxOGZlYWI5MmE4ZjViZWI1OWI5ZGRmNTdmYWMwNWJjN2VkOWEwOTM)

=============

### Build status

![CI](https://github.com/RaftLib/RaftLib/workflows/CI/badge.svg?event=push)

### Pre-requisites

#### OS X & Linux
Compiler: c++14 capable -> Clang, GNU GCC 5.0+, or Intel icc
Libraries: 
* Boost, if not installed, needed headers automatically downloaded with cmake

#### Windows
* Latest merge from pull request to main should enable compilation on VS on Win10.

### Install
Make a build directory (for the instructions below, we'll 
write [build]). If you want to build the OpenCV example, then
you'll need to add to your cmake invocation:
```bash
-DBUILD_WOPENCV=true 
```

To use the [QThreads User space HPC threading library](http://www.cs.sandia.gov/qthreads/) 
you will need to add the following (NOTE: The qthread library currently uses its own partitioner
and does not work with Scotch, it also has issues with OpenCV, will fix in next release 
iteration):
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
* [Project web page](http://raftlib.io)
* [Project wiki page](https://github.com/jonathan-beard/RaftLib/wiki)
* [Blog post intro](https://goo.gl/4VDlbr)
* [Jonathan Beard's thesis](http://goo.gl/obkWUh)
* [Views on parallel computing, general philosphy](https://goo.gl/R5fQAl)
* Feel free to e-mail one of the authors of the repo
