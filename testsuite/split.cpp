#include <raft>
#include <cstdint>
#include <iostream>
#include <raftio>
#include "generate.tcc"

int
main( int argc, char **argv )
{
   using namespace raft;
   using gen = raft::test::generate< std::int64_t >;
   const auto count( 10000 );
   /** manually link split kernels **/
   auto kernels( 
   map.link( kernel::make< gen >( count ),
             kernel::make< split< std::int64_t > >() ) );
   
   map.link( &kernels.getDst(), 
             kernel::make< print< std::int64_t, '\n' > >() );
   map.exe();
   return( EXIT_SUCCESS );
}
