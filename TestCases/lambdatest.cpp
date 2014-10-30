#include <raft>
#include <raftio>

#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <random>

int
main( int argc, char **argv )
{
   std::default_random_engine generator;
   std::uniform_int_distribution<int> distribution(1,6);
   auto rand_func = std::bind( distribution,  generator ); 
   std::size_t gen_count( 0 );
   using namespace raft;

   map.link( 
      kernel::make< lambdak< int > >( 0, 1, [&]( Port &input,
                                                 Port &output )
                                            {
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

   return( EXIT_FAILURE );
}
