## Simple Test Files

To compile copy the package configuration file (raftlib.pc) or the qthreads
version to the pkg-config dir. At some point we'll automate this with the 
cmake build, but for now it's manual. 

Usually the path is something like 
```bash
/usr/lib/x86_64-linux-gnu/pkgconfig
```

Once that's done you should be able to do 
```bash
~/GIT_RPO/RaftLib/examples/simple$ pkg-config --list-all | grep raftlib
raftlib                        raftlib - RaftLib C++ Streaming/Data-flow Library
```

Now when compiling from the command line (assuming your libraries are
installed in the default locations (/usr/local/XX) then you can 
do this from the command line, assuming you replace your filenames
with the appropriate ones. 

```bash
g++ `pkg-config --cflags raftlib` -o <name> <file>.cpp `pkg-config --libs raftlib`
```
