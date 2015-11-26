#include <raft>
#include <cstdint>
#include <iostream>
#include <raftio>
#include <raftrandom>


int
main( int argc, char **argv )
{
   using type  = std::uint32_t;
   using gen   = raft::random_variate< type, 
                                       raft::sequential >;
   using join  = raft::join< type >;
   using print = raft::print< type, '\n' >;

   /** manually link split kernels **/
   auto kernels( 
   raft::map.link( raft::kernel::make< gen >( 1, 10000 ),
                   raft::kernel::make< join >() ) );
   
   raft::map.link( &kernels.getDst(), 
                   raft::kernel::make< print >() );
   raft::map.exe();
   return( EXIT_SUCCESS );
}
