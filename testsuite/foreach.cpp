#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <vector>
#include <iterator>
#include <raft>
#include <raftio>

int
main()
{
   const auto arr_size( 1000 );
   using type_t = std::int32_t;
   type_t *arr = (type_t*) malloc( sizeof( type_t ) * arr_size );
   for( type_t i( 0 ); i < arr_size; i++ )
   {
      arr[ i ] = i;
   }
   
   using print    = raft::print< type_t, '\n' >;
   using foreach  = raft::for_each< type_t >;
   print   p;
   foreach fe( arr, arr_size, 1 );
   raft::map m;
   m += fe >> p;
   m.exe();
   free( arr );
   return( EXIT_SUCCESS );
}
