#include "raftmanip.hpp"
#include "kernel.hpp"

void 
raft::_local::apply_help( const manip_vec_t value, raft::kernel &k )
{
    k.apply( value );
}
