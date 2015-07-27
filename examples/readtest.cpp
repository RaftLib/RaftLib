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
   raft::map.link( 
      raft::kernel::make< fr > ( argv[ 1 ]  ), 
      raft::kernel::make< pr >() );
   raft::map.exe();
   return( EXIT_SUCCESS );
};
