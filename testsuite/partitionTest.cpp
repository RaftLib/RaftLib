#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>
#include "generate.tcc"

int
main( int argc, char **argv )
{
   int count( 1000 );
   if( argc == 2 )
   {
      count = atoi( argv[ 1 ] );
   }
   auto rndgen( raft::kernel::make< 
      raft::test::generate< std::uint32_t > >( count ) );
                                                              
   
   using sub = raft::lambdak< std::uint32_t >;
   auto  l_sub( [&]( Port &input,
                     Port &output )
      {
         std::uint32_t a;
         input[ "0" ].pop( a );
         output[ "0" ].push( a - 10 );
         return( raft::proceed );
      } );

    
   auto kernels = raft::map.link( rndgen,
                                  raft::kernel::make< sub >( 1, 1, l_sub ) );
   
   for( int i( 0 ); i < 50; i++ )
   {
      kernels = raft::map.link( &kernels.getDst(),
                                raft::kernel::make< sub >( 1, 1, l_sub ) );
   }
   raft::map.link( &kernels.getDst(), 
             raft::kernel::make< raft::print< std::uint32_t, '\n' > >() );
   raft::map.exe();
   

   return( EXIT_SUCCESS );
}
