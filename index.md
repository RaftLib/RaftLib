### Welcome to the Raft C++ Stream Processing Template Library

### What is it?
[Stream processing](http://www.jonathanbeard.io/blog/2015/09/19/streaming-and-dataflow.html) is a method for extracting data, task, and pipeline parallelism from an application. Ever wash dishes with more than one person, one washer, one dryer, and perhaps one person to put them away? That's pipeline parallelism. Raft lets you do that for your application. How about add two washers, three? You can do this as well. That's data parallelism, you're operating on differing (independent) dishes at the same time. Going further, the Raft library enables task parallelism. That is, if you have four people on the dishes, you can add somebody on the side, taking inventory of the dishes as the pass by, at the same time the dishes are being washed. The inventory task is independent of the other two (which is slightly more nuanced since we've to spot the dishes as they go by). The Raft library uses templates, and a compiled library, to make the otherwise perilous task of creating parallel programs far easier. Please check it out, and if you have any questions, please feel free to send the project developers an email.

### Project Status
The Raft library is currently an alpha release, nearing its first beta.  The API itself is relatively stable, however the back end
is subject to updates if needed (although we're taking pains to support needed old features). This software is subject to the Apache Version 2.0 license. The main-line branch is also the development branch. As new code is checked in, it is processed through two continuous integration services, the build status icon reflects the current Linux build and testsuite success. You can add applications to the main line through subsequent pull requests. There are also several releases, at this stage of the project, snapshots for releases are taken before any major changes. We plan on moving to a full beta release once the multi-node support is pulled from the research code and re-integrated with the mainline branch (at the current rate of development, that should be mid-March 2016). 

### Build Status

[![Build Status](https://travis-ci.org/jonathan-beard/RaftLib.svg?branch=master)](https://travis-ci.org/jonathan-beard/RaftLib)

### Setup
To checkout the library and build RaftLib (along with all examples) do the following:
```
$ cd location_for_the_repo
$ git clone git@github.com:RaftLib/RaftLib.git
$ mkdir build
$ cd build
$ cmake ../RaftLib
$ make & make test
$ sudo make install
```

Additionally, if you would like to build the OpenCV example, add 
```bash
-DBUILD_WOPENCV=true
```
to your cmake invocation, you will need a webcam or video source for it to work.

The make command will generate the static library file _libraft.a_, as well as test cases and all the example applications. These examples can be found in:

```
$ cd build/examples
```  

To install the library to your system (in the default prefix), simply run 
```bash
sudo make install
```

## How to build apps and use RaftLib
When building an application using the library the only header file needed is the _raft_ header (see wiki for details), and you must link with the raft library file created during the compilation phase above. Many more details can be found in the following references (we'll endeavor to keep these up to date, but please feel free to contribute to the wiki or point out errors on our part):

* [Project Wiki](https://github.com/jonathan-beard/RaftLib/wiki)
* [Project README](https://github.com/jonathan-beard/RaftLib/blob/master/README.md)
* [Blog post intro](https://goo.gl/4VDlbr)
* [Jonathan Beard's thesis](http://goo.gl/obkWUh)
* [Parallel Bzip2 Example](https://medium.com/cat-dev-urandom/simplifying-parallel-applications-for-c-an-example-parallel-bzip2-using-raftlib-with-performance-f69cc8f7f962)

### Documentation
The main source of documentation is available on the [RaftLib wiki page](https://github.com/jonathan-beard/RaftLib/wiki)

If you see errors, or want to contribute please feel free to do so. 

### Authors and Contributors
In 2013 Jonathan Beard (@jonathan-beard, also <a href="https://twitter.com/jonathan_beard" class="twitter-follow-button" data-show-count="false">Follow @jonathan_beard</a> <script>!function(d,s,id){var js,fjs=d.getElementsByTagName(s)[0],p=/^http:/.test(d.location)?'http':'https';if(!d.getElementById(id)){js=d.createElement(s);js.id=id;js.src=p+'://platform.twitter.com/widgets.js';fjs.parentNode.insertBefore(js,fjs);}}(document, 'script', 'twitter-wjs');</script> ) started work on the Raft language.  In the interim,
and realizing the ubiquity of the C++ language, he started work on a template library that 
uses the same framework as the full Raft language and run-time system.  This C++ library is 
the result. Since then many have contributed to the theory and code base behind RaftLib (most
notable is my co-author (@pl53), for a full current contribution list see the GitHub stats page. 

### Citing the Raft Library
Please cite as:
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

### Genealogy
There have been several dozen streaming languages.  Most notable of which is probably StreaMIT.  I've worked on the Auto-Pipe streaming runtime system (both versions 1 and 2) developed at Washington University.  This particular streaming library was developed as a faster way to get started with stream processing by enabling users to stick to a language with which they are already familiar (C++).


### Issues
Issues can be opened by filing a report on our [issues page](https://github.com/jonathan-beard/RaftLib/issues)
