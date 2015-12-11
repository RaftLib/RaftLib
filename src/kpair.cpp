#include "kpair.hpp"
#include "kernel.hpp"


kpair::kpair( raft::kernel &a, raft::kernel &b )
{
    src = &a;
    src_name = a.getEnabledPort();
    dst = &b;
    dst_name = b.getEnabledPort();
}

kpair operator << ( raft::kernel &&a, raft::kernel &&b )
{
    return( kpair( a, b ) );
}
