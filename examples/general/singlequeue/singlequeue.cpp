/**
 * app just forms a single queue, prints a lot of numbers. 
 */
#include <cassert>
#include <iostream>
#include <cstdint>
#include <cstdlib>
#include <raft>
#include <raftio>
#include <raftrandom>



int
main()
{
    using type_t = std::int64_t;
    using gen = raft::random_variate< std::mt19937_64,
                                      std::uniform_int_distribution,
                                      type_t >;
    using pr  = raft::print< type_t, '\n' >;
    raft::map m;
    gen g( 100000 );
    pr  print;
    m += g >> print;
    m.exe();
    return( EXIT_SUCCESS );
}
