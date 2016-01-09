#include <cassert>

#include "kpair.hpp"
#include "kernel.hpp"


kpair::kpair( raft::kernel &a, 
              raft::kernel &b,
              const bool split,
              const bool join ) : kpair( a, b )
{
    split_to = split;
    join_from = join;
}

kpair::kpair( kpair * const prev, 
              raft::kernel &b,
              const bool split,
              const bool join ) : kpair( *prev->dst, b )
{
    head        = prev->head;
    prev->next  = this;
    split_to    = split;
    join_from   = join;
}

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
    src_out_count = a.output.count();
    dst_in_count  = b.input.count();
    head = this;
}

kpair* 
operator >> ( raft::kernel &a, raft::kernel &b )
{
    return( new kpair( a, b ) );
}

kpair*
operator >> ( raft::kernel &&a, raft::kernel &&b )
{
    return( new kpair( a, b ) );
}

kpair*  
operator >> ( kpair *a, raft::kernel &b )
{
    return( new kpair( a, b, false, false ) );
}

kpair*  
operator >> ( kpair *a, raft::kernel &&b )
{
    return( new kpair( a, b, false, false ) );
}

kpair*
operator <= ( raft::kernel &a, raft::kernel &b )
{
    return( new kpair( a, b, true, false ) );
}

kpair*
operator <= ( raft::kernel &&a, raft::kernel &&b )
{
    return( new kpair( a, b, true, false ) );
}

kpair*
operator >= ( kpair *a, raft::kernel &&b )
{
    return( new kpair( a, b, false, true ) );
}

kpair*
operator >= ( kpair *a, raft::kernel &b )
{
    return( new kpair( a, b, false, true ) );
}


