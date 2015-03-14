#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>
#include <raftrandom>


int
main( int argc, char **argv )
{
   auto rndgen( raft::kernel::make< 
      raft::random_variate< std::uint32_t, raft::uniform > >( 100, 
                                                              10000, 
                                                              1000000 ) );
   
   using sub = raft::lambdak< std::uint32_t >;
   auto  l_sub( [&]( Port &input,
                     Port &output )
      {
         auto a( input[ "0" ].pop_s< std::uint32_t >() );
         output[ "0" ].push( (*a) - 1 );
         return( raft::proceed );
      } );

   
   auto kernels = raft::map.link( rndgen,
                                  raft::kernel::make< sub >( 1, 1, l_sub ) );
   for( int i( 0 ); i < 20; i++ )
   {
      kernels = raft::map.link( &kernels.getDst(),
                                raft::kernel::make< sub >( 1, 1, l_sub ) );
   }
   raft::map.link( &kernels.getDst(), 
             raft::kernel::make< raft::print< std::uint32_t, '\n' > >() );
   raft::map.exe();
   return( EXIT_SUCCESS );
}
