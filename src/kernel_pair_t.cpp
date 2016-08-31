#include "kernel_pair_t.hpp"

//TODO, finish me
kernel_pair_t::kernel_pair_t()
{
    source.reserve( 2 );
    destination.reserve( 2 );
}

kernel_pair_t::kernel_pair_t( raft::kernel * const src,
                              raft::kernel * const dst ) : kernel_pair_t()
{
    source.emplace_back( *src );
    destination.emplace_back( *dst );
}
