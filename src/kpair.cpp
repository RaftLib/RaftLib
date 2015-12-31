#include "kpair.hpp"
#include "kernel.hpp"

kpair::kpair( raft::kernel &a, raft::kernel &b )
{
    src = &a;
    src_name = a.getEnabledPort();
    if( src_name.length() > 0 )
    {
        /** set false by default **/
        has_src_name = true;
    }
    dst = &b;
    dst_name = b.getEnabledPort();
    if( dst_name.length() > 0 )
    {
        /** set false by default **/
        has_dst_name = true;
    }
}

kpair 
operator >> ( raft::kernel &a, raft::kernel &b )
{
    return( kpair( a, b ) );
}

kpair 
operator >> ( raft::kernel &&a, raft::kernel &&b )
{
    return( kpair( a, b ) );
}
