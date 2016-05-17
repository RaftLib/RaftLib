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
   if( argc < 2 )
   {
       std::cerr << "must have at least 1 argument passed to: " << argv[ 0 ] << "\n";
       exit( EXIT_FAILURE );
   }
   fr reader( argv[ 1 ] );
   pr printer;

   raft::map m;
   m += reader >> printer;
   m.exe();

   return( EXIT_SUCCESS );
}
