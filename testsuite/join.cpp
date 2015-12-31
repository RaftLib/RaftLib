#include <raft>
#include <cstdint>
#include <iostream>
#include <raftio>
#include "generate.tcc"

int
main( int argc, char **argv )
{
   using type  = std::uint32_t;
   using gen   = raft::test::generate< type >;
   using join  = raft::join< type >;
   using print = raft::print< type, '\n' >;

   gen      g( 10000 );
   join     j;
   print    p;
   raft::map m;
   /** manually link split kernels **/
   m += g >> j;
   m += j >> p;
   m.exe();
   
   return( EXIT_SUCCESS );
}
