#include <raft>
#include <raftio>

#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <random>

int
main( int argc, char **argv )
{
   using namespace raft;
   Map map;

   map.link( 
      kernel::make< lambdak< int > >( 0, 1, [&]( Port &input,
                                                 Port &output )
                                            {
   static std::default_random_engine generator;
   static std::uniform_int_distribution<int> distribution(1,10);
   static auto rand_func = std::bind( distribution,  generator ); 
   static std::size_t gen_count( 0 );
                                                if( gen_count++ < 10000 )
                                                {
                                                   output[ "0" ].push( 
                                                      rand_func(),
                                                      raft::none );
                                                   return( raft::proceed );
                                                }
                                                return( raft::stop );
                                            } ),
      kernel::make< print< int, '\n' > >() );
   map.exe();

   return( EXIT_SUCCESS );
}
