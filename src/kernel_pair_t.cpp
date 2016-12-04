#include "kernel_pair_t.hpp"

kernel_pair_t::kernel_pair_t()
{
    /** TODO, might need to optimize with something better **/
    source.reserve( 2 );
    destination.reserve( 2 );
}

kernel_pair_t::kernel_pair_t( raft::kernel * const src,
                              raft::kernel * const dst ) : kernel_pair_t()
{
    source.emplace_back( *src );
    destination.emplace_back( *dst );
}


kernel_pair_t::kernel_pair_t( raft::kernel &src,
                              raft::kernel &dst ) : kernel_pair_t()
{
    source.emplace_back( src );
    destination.emplace_back( dst );
}

kernel_pair_t::endpoint_ret_type
kernel_pair_t::getSrc()
{
    return( endpoint_ret_type( source.begin(), source.end() ) );
}

kernel_pair_t::size_type
kernel_pair_t::getSrcSize() noexcept
{
    return( source.size() );
}


kernel_pair_t::endpoint_ret_type
kernel_pair_t::getDst()
{
    return( endpoint_ret_type( destination.begin(), destination.end() ) );
}

kernel_pair_t::size_type
kernel_pair_t::getDstSize() noexcept
{
    return( destination.size() );
}

void
kernel_pair_t::addSrc( raft::kernel &k ) noexcept
{
    source.emplace_back( k );
}

void
kernel_pair_t::addDst( raft::kernel &k ) noexcept
{
    destination.emplace_back( k );
}

void
kernel_pair_t::clearSrc() noexcept
{
    source.clear();
}

void
kernel_pair_t::clearDst() noexcept
{
    destination.clear();
}
