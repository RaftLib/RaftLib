#include <raft>
#include <cstdint>
#include <iostream>
#include <raftio>
#include <raftrandom>

int
main( int argc, char **argv )
{
   using namespace raft;
   using gen = raft::random_variate< std::int64_t, raft::sequential >;
   const auto count( 10000 );
   /** manually link split kernels **/
   auto kernels( 
   map.link( kernel::make< gen >( 1    /* start range */, 
                                  1000 /* end range */, 
                                  1    /* delta */, 
                                  count ),
             kernel::make< split< std::int64_t > >() ) );
   
   map.link( &kernels.getDst(), 
             kernel::make< print< std::int64_t, '\n' > >() );
   map.exe();
   return( EXIT_SUCCESS );
}
