#include <raft>
#include <raftio>

#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <random>
#include "defs.hpp"

int
main()
{
    using type_t = std::int32_t;
    using print  = raft::print< type_t, '\n' >;
    using rnd    = raft::lambdak< type_t >;
    rnd lk( /** input ports     **/ 0, 
            /** output ports    **/ 1, 
            /** the kernel func **/
        [&]( Port &input,
             Port &output )
        {
            UNUSED( input );
            static std::default_random_engine generator;
            static std::uniform_int_distribution< type_t > distribution(1,10);
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
        } );

    print p;
    raft::map m;

    m += lk >> p;
    m.exe();
    return( EXIT_SUCCESS );
}
