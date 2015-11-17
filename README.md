

=============
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

### Install
Once the pre-requisite libraries are installed
then go to the root RaftLib directory and type
```
cmake .
make
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
the sub-modules sometime this week (16 Nov. 2015)
