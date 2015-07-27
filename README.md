
=============
### Pre-requisites
####OS X
Compiler: Intel icc or GNU GCC 4.8+
Libraries: <a href="http://goo.gl/gchdSw">GNU Scientific Library</a>
####Linux
Compiler: C++11 capable 
Libraries: <a href="http://goo.gl/gchdSw">GNU Scientific Library</a>

## NOTE:
Scotch or Metis will probably return as partitioning packages in future
releases, however for the moment I've disabled them since I think I can
do better by focusing on the special cases instead of partitioning for
the general case.  

### Install
Once the pre-requisite libraries are installed
then go to the root RaftLib directory and type
```
make 
sudo make install
```
NOTE: The default prefix in the makefile is 
```
PREFIX ?= /usr/local
```
change via environment variable or in the Makefile to install
somewhere else.

### Getting Started
There are a few examples in the ./examples folder.  It might
also be useful to take a look at the testsuite (named as such
since I'm eventually going to use GNU's auto tools).  

Academic Papers (not all implemented in the release/main branch), see references
1-6 @ <a href="http://www.jonathanbeard.io/publications/">pubs</a>.  If you use
for academic work, please cite:

<span id="bc15b">Beard, J. C., Li, P., &amp; Chamberlain, R. D. (2015). RaftLib: A C++ template library for high performance stream parallel processing. In <i>Proceedings of Programming Models and Applications on Multicores and Manycores</i>. New York, NY, USA: ACM.</span>

A wiki page is coming soon!


