
=============
### Pre-requisites
####OS X
Compiler: Intel icc or GNU GCC 4.8+
Libraries: <a href="http://goo.gl/gchdSw">GNU GSL</a> and <a href="http://goo.gl/tI1NGf">Scotch</a>
####Linux
Compiler: c++11 capable 
Libraries: <a href="http://goo.gl/gchdSw">GNU GSL</a> and <a href="http://goo.gl/tI1NGf">Scotch</a>

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
