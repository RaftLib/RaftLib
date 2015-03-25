#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>
#include <raftrandom>
#include <fstream>

int
main( int argc, char **argv )
{
   int count( 1000 );
   if( argc == 2 )
   {
      count = atoi( argv[ 1 ] );
   }
   auto rndgen( raft::kernel::make< 
      raft::random_variate< std::uint32_t, raft::uniform > >( 1000, 
                                                              2000, 
                                                              count  ) );
                                                              
   
   using sub = raft::lambdak< std::uint32_t >;
   auto  l_sub( [&]( Port &input,
                     Port &output )
      {
         if( output[ "0" ].space_avail() > 0 )
         {
            std::uint32_t a;
            input[ "0" ].pop( a );
            output[ "0" ].push( a - 10 );
         }
         return( raft::proceed );
      } );

   //std::ofstream ofs( "/dev/null" );
    
   auto kernels = raft::map.link( rndgen,
                                  raft::kernel::make< sub >( 1, 1, l_sub ) );
   
   for( int i( 0 ); i < 50 ; i++ )
   {
      kernels = raft::map.link( &kernels.getDst(),
                                raft::kernel::make< sub >( 1, 1, l_sub ) );
   }
   raft::map.link( &kernels.getDst(), 
             raft::kernel::make< raft::print< std::uint32_t, '\n' > >( std::cout ) );
   raft::map.exe();
   
   //ofs.close();

   return( EXIT_SUCCESS );
}
