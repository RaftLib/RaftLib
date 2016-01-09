#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>
#include <raftrandom>
#include <raftmath>

template< typename T  > class Mult : public raft::kernel
{
public:
   Mult() : raft::kernel()
   {
      input.template addPort< T >( "a", "b" );
      output.template addPort< T  >( "mult" );
   }
   
   virtual raft::kstatus run()
   {
      auto out( output[ "mult" ].template allocate_s< T >() );
      (*out) = std::move( raft::mult< T >( input[ "a" ], input[ "b" ] ) );
      return( raft::proceed );
   }

};


int
main( int argc, char **argv )
{
    int count( 1000 );
    if( argc == 2 )
    {
       count = atoi( argv[ 1 ] );
    }
    using type_t = std::int64_t;
    using gen = raft::random_variate< std::mt19937_64,
                                      std::uniform_int_distribution,
                                      type_t >;
    using mult = Mult< type_t >;
    using p_out = raft::print< type_t, '\n' >;

    raft::map m;
    gen     a( count ), b( count );
    mult    multiplier;
    p_out   printer;
    m += a >> multiplier[ "a" ];
    m += b >> multiplier[ "b" ];
    m += multiplier >> printer;
    m.exe();
    return( EXIT_SUCCESS );
}
