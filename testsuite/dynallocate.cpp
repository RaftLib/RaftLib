#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <vector>
#include <functional>
#include <iostream>
#include <raft>
#include <cstdlib>
#include <chrono>
#include <thread>
#include <raftio>
//test sequential num generator
#include "generate.tcc"

//test case just generates lots of prints,
//exercises queue re-allocation scheme
int
main( int argc, char **argv )
{
   /** set aliases for kernel types **/
   using gen = raft::test::generate< std::int64_t >;
   using p   = raft::print< std::int64_t, '\n' >;
   gen a( 1000000 );
   p   print;
   raft::map m;
   m += a >> print;
   m.exe();

   return( EXIT_SUCCESS );
}



