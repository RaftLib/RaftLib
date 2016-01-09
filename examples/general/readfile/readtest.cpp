#include <cstdlib>
#include <iostream>
#include <raft>
#include <raftio>

int
main( int argc, char **argv )
{
   using chunk = raft::filechunk< 1024 >;
   using fr = raft::filereader< chunk, false >; 
   using pr = raft::print< chunk >;
   fr reader( argv[ 1 ] );
   pr printer;

   raft::map m;
   m += reader >> printer;
   m.exe();
   
   return( EXIT_SUCCESS );
};
