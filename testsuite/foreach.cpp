#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <iterator>
#include <raft>
#include <raftio>

int
main( int argc, char **argv )
{
   const auto arr_size( 1000 );
   std::int32_t *arr = (std::int32_t*) malloc( sizeof( std::int32_t ) * arr_size );
   for( std::int32_t i( 0 ); i < arr_size; i++ )
   {
      arr[ i ] = i;
   }
   
   using print    = raft::print< std::int32_t, '\n' >;
   using foreach  = raft::for_each< std::int32_t >;

   raft::map.link( 
      raft::kernel::make< foreach >( arr, arr_size, 1),
      raft::kernel::make< print   >() );

   raft::map.exe();
   
   free( arr );
   return( EXIT_SUCCESS );
}
