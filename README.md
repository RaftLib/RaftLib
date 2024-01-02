# Raftlib [![Test](https://github.com/RaftLib/RaftLib/actions/workflows/test.yml/badge.svg)](https://github.com/RaftLib/RaftLib/actions/workflows/test.yml) [![Release](https://github.com/RaftLib/RaftLib/actions/workflows/release.yml/badge.svg)](https://github.com/RaftLib/RaftLib/actions/workflows/release.yml)


## Overview

[RaftLib](http://raftlib.io) is an open-source C++ Library that provides a framework for implementing parallel and concurrent data processing pipelines. It is designed to simplify the development of high-performance data processing applications by abstracting away the complexities of parallelism, concurrency, and data flow management.

It enables stream/data-flow parallel computation by linking parallel compute kernels together using simple right shift operators, similar to C++ streams for string manipulation. RaftLib eliminates the need for explicit usage of traditional threading libraries such as pthreads, std::thread, or OpenMP, which can lead to non-deterministic behavior when misused.

RaftLib's model provides a lock-free FIFO-like access to communication channels that connect each compute kernel. This design ensures efficient and deterministic data transfer between kernels. The library incorporates auto-parallelization, optimization, and convenient features to simplify the development of performant applications.

Key Components:
1. **Pipelines**: The core concept in RaftLib is a pipeline, which represents a sequence of data processing stages. Each stage in the pipeline is implemented as a separate computational unit called a "kernel." Kernels are connected together to form a directed acyclic graph (DAG) that represents the data flow between stages.

2. **Kernels**: Kernels are the building blocks of a pipeline and encapsulate the computation performed on the input data. Each kernel can have one or more input ports and output ports, allowing data to flow between stages. Kernels are responsible for processing data, applying transformations, and generating output for downstream stages.

3. **Schedulers**: RaftLib provides different schedulers to control the execution and parallelism of the pipeline. Schedulers determine how and when kernels are executed, taking into account factors like data dependencies, load balancing, and available computational resources. The library offers various scheduling strategies, including static scheduling, dynamic scheduling, and hybrid scheduling.

4. **Data Flow Management**: RaftLib handles the data flow between kernels automatically. It manages the movement of data between stages, ensuring that input data is available when needed and that output data is delivered to the correct destination. The library provides mechanisms for handling backpressure and buffering, allowing efficient processing of data streams.

5. **Parallelism and Concurrency**: RaftLib enables parallel execution of pipeline stages by utilizing the available computational resources efficiently. It supports multi-threading and takes advantage of multiple CPU cores to achieve parallelism. Additionally, it can leverage GPU acceleration for certain kernels, further boosting performance.

6. **Integration and Extensibility**: RaftLib provides an API and set of tools for integrating the library into existing applications. It supports interoperability with other libraries and frameworks, making it possible to combine RaftLib with domain-specific tools. The library is extensible, allowing developers to define custom kernels and schedulers to fit specific application requirements.

7. **Fault Tolerance**: RaftLib offers mechanisms for handling failures and recovering from errors. It supports fault-tolerant execution by providing checkpointing and recovery capabilities, allowing pipelines to resume from a previous state in case of failures.

Overall, RaftLib simplifies the development of parallel and concurrent data processing applications by providing a high-level abstraction for building data flow pipelines. It allows developers to focus on the computation and data transformations, while the library handles the complexities of parallel execution, data flow management, and fault tolerance.

Feel free to give RaftLib a try! If you encounter any issues, please create an issue request. For minor issues, we recommend joining our Slack group for quick resolutions. We also welcome pull requests from the community! If you're interested in benchmarking, you can send the authors an email. We have started a benchmark collection, but it's a work in progress, and we would be delighted to include your code.

User Group / Mailing List: [slack channel](https://join.slack.com/t/raftlib/shared_invite/zt-3sk6ms6f-eEBd23dz98JnoRiXLaRmNw)


## Pre-requisites

### Linux
* Compiler: c++17 capable -> Clang, GNU GCC 5.0+, or Intel icc
* Latest build runs under Linux with above compilers on both x86 and AArch64, with both pthreads and QThreads. 

### OS X
* Compiler: c++17 capable -> Clang, GNU GCC 5.0+, or Intel icc
* OS X M1 runs, compiles, but has some test case hiccups on templates, but doesn't seem to 
impact functionality. 
* Note for OS X users without a /user/local, specify an install prefix when using CMake. 

### Windows
* Builds and runs under Win10


## Cloning repository
Clone using the --recurse-submodules to download the library including all submodules and other libraries

```git clone --recurse-submodules https://github.com/RaftLib/RaftLib.git```

## Downloading Manually

Building the library by cloning the repository is the preferred option. However, when this cannot be done, such as in offline networks, manual downloading of the package is necessary. In such cases, we must also ensure to manually download the corresponding dependencies.

### Dependencies

The following submodules are required for building RaftLib, and they need to be placed under their corresponding folders within the `git-dep` directory:

- [**affinity**](https://github.com/RaftLib/affinity) - Provides CPU affinity setting capabilities.
  - Commit: Specify the required commit here.

- [**cmdargs**](https://github.com/RaftLib/cmdargs) - Offers command-line argument parsing functionality.
  - Commit: Specify the required commit here.

- [**demangle**](https://github.com/RaftLib/demangle) - Facilitates C++ symbol demangling.
  - Commit: Specify the required commit here.

- [**shm**](https://github.com/RaftLib/shm) - Supports shared memory communication.
  - Commit: Specify the required commit here.

Before building RaftLib, ensure that you download the corresponding commit of each submodule. You can use the following command within the main repository:
```bash
git submodule update --init --recursive
```

After setting up the dependencies, you can proceed with building and using RaftLib as described.

## Build and Install
Using a build directory called e.g.: "build": 

```bash
mkdir build
cd build
cmake ..
make && make test
sudo make install
```
NOTE: The default prefix in the makefile is: 
```
PREFIX ?= /usr/local
```


## CMAKE flags
### OpenCV
If you want to build the OpenCV example, then
you'll need to add to your cmake invocation:
```bash
-DBUILD_WOPENCV=true 
```

### Examples
Building the examples can be enabled using:
```bash
-DBUILD_EXAMPLES=true
```

### Benchmarks
Building the benchmarks can be enabled using:
```bash
-DBUILD_BENCHMARKS=false
```

### Tests
Building tests can be disabled using:
```bash
-DBUILD_TESTS=false
```

### QThreads
To use the [QThreads User space HPC threading library](http://www.cs.sandia.gov/qthreads/) 
you will need to use the version with the RaftLib org and follow the RaftLib specific readme. 
This QThreads version has patches for hwloc2.x applied and fixes for test cases. To compile
RaftLib with QThreads linked, add the following (assumes the QThreads library is in your path):
```bash
-DUSEQTHREAD=1
```

### String names
This is still an experimental feature. Default is to use legacy string-named ports.
```bash
-DSTRING_NAMES=1
```

### Pkg-config path
Set the pkg-config path where to install the `raftlib.pc` configuration file. Leave empty for the application to figure it out.
```bash
-DPKG_CONFIG_PATHWAY="<path>"
```

### Generate position independent code
Sometimes the code needs to be integrated into a shared library, for that this flag allows building the library with position independet code (i.e.: with the compiling flag -fPIC supported by both gcc and clang)
```bash
-DBUILD_FPIC=1
```


## Using
When building applications with RaftLib, on Linux it is best to 
use the **pkg-config** file, as an example, using the _poc.cpp_ example,
```bash
g++ `pkg-config --cflags raftlib` poc.cpp -o poc `pkg-config --libs raftlib`
```

Feel free to substitute your favorite build tool. I use Ninja and make depending on which machine I'm on. To change out, use cmake to generate the appropriate build files with the -Gxxx flag.

## Pkg-config
The primary use of pkg-config is to provide the necessary details for compiling and linking a program to a library. This metadata is stored in pkg-config files. These files have the suffix .pc and reside in specific locations known to the pkg-config tool. RaftLib provides a configuration which which is installed together with the library. Once the configuration file is installed, the command `pkg-config --cflags raftlib` can be used to provide the compiling details.

Following is an example of what is returned by above command:
```bash
-std=c++14 -DL1D_CACHE_LINE_SIZE=64 -DPLATFORM_HAS_NUMA=0 -I/usr/local/include
```

## Contribution Guidelines

We welcome contributions to our project! To maintain a clear and organized development history, please adhere to the Conventional Commits message format when making commits.

### Conventional Commits

Please follow the guidelines from the [Conventional Commits website](https://www.conventionalcommits.org/) when crafting your commit messages. This format helps us generate accurate changelogs and automate the release process based on the types of changes you make.

## Automatic Releases

We've streamlined our release process to be automated, thanks to the Conventional Commits message format. This ensures that our project maintains a clear versioning scheme and changelog, without the need for manual intervention.

### How Automatic Releases Work

When you follow the Conventional Commits message format for your commit messages, our automated release system interprets these messages and determines the appropriate version bump for the project.

- Commits with **fix:** in the message trigger a **patch** version increase.
- Commits with **feat:** in the message trigger a **minor** version increase.
- Commits with a **BREAKING CHANGE:** in the message trigger a **major** version increase.

Here's an example of how it works:
- If you contribute a bug fix, such as `fix: resolve login issue`, it will trigger a patch version increase.
- If you add a new feature, such as `feat: implement user profile customization`, it will trigger a minor version increase.
- If your contribution includes a breaking change, such as `BREAKING CHANGE: update authentication method`, it will trigger a major version increase.

### Benefits of Automated Releases

Automated releases offer several benefits to our development workflow:
- **Consistency:** Every release follows a standardized versioning scheme.
- **Changelog Generation:** Changelogs are automatically generated based on commit messages.
- **Efficiency:** Release management is streamlined, saving time and reducing errors.
- **Transparency:** Contributors can see how their changes affect the versioning process.

By adhering to the Conventional Commits format, you play a crucial role in ensuring that our project's releases are accurate, well-documented, and hassle-free.

Thank you for your contributions and for helping us maintain a smooth and automated release process!

### Warning

We do not support exclamation marks (`!`) after `<type>` for triggering breaking changes.

## Citation

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

## Other Info Sources
* [OpenCV wrappers for RaftLib](https://github.com/RaftLib/RaftOCV)
* [Project web page](http://raftlib.io)
* [Project wiki page](https://github.com/jonathan-beard/RaftLib/wiki)
* [Blog post intro](https://goo.gl/4VDlbr)
* [Jonathan Beard's thesis](http://goo.gl/obkWUh)
* [Views on parallel computing, general philosphy](https://goo.gl/R5fQAl)

Feel free to e-mail one of the authors of the repo
 
