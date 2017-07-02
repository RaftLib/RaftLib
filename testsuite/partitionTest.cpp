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
                                                               
    raft::test::generate< std::uint32_t > rndgen( count );
    raft::print< std::uint32_t, '\n' > p;

    using sub = raft::lambdak< std::uint32_t >;
    auto  l_sub( []( Port &input,
                     Port &output ) -> raft::kstatus
       {
          std::uint32_t a;
          input[ "0" ].pop( a );
          output[ "0" ].push( a - 10 );
          return( raft::proceed );
       } );

    raft::map m;
    /** make one sub kernel, this one will live on the stack **/
    sub s( 1, 1, l_sub );
    kernel_pair_t::kernel_iterator_type BEGIN, END;
    auto kernels( m += rndgen >> s );
    for( int i( 0 ); i < 
#ifdef USEQTHREADS
    1000
#else
    5
#endif
    ; i++ )
    {
        std::tie( BEGIN, END ) = kernels.getDst();
        kernels = ( m += (*BEGIN).get() >> raft::kernel::make< sub >( 1, 1, l_sub ) ); 
    }
    std::tie( BEGIN, END ) = kernels.getDst();
    m += (*BEGIN).get() >> p;
    m.exe();
    return( EXIT_SUCCESS );
}
