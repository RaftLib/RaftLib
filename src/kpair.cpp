#include <cassert> 
#include <utility>
#include "portexception.hpp"
#include "kpair.hpp"
#include "kernel.hpp"
#include "portorder.hpp"
#include "kernel_wrapper.hpp"

kpair::kpair( raft::kernel &src, 
              raft::kernel &dst,
              const bool split,
              const bool join ) : kpair( src, dst )
{
    split_to = split;
    join_from = join;
}

kpair::kpair( raft::kernel &src, 
              raft::kernel_wrapper &dst,
              const bool split,
              const bool join ) : kpair( src, *(*dst), split, join )
{}

kpair::kpair( raft::kernel_wrapper &src, 
              raft::kernel &dst,
              const bool split,
              const bool join ) : kpair( *(*src), dst, split, join )
{}

kpair::kpair( raft::kernel_wrapper &src, 
              raft::kernel_wrapper &dst,
              const bool split,
              const bool join ) : kpair( *(*src), *(*dst), split, join )
{}

/**
 * kpair - for joining kpair on the left (src)
 * and dst on right.
 */
kpair::kpair( kpair &src, 
              raft::kernel &dst,
              const bool split,
              const bool join ) : kpair( *(src.out.k), dst )
{
    head        = src.head;
    src.next  = this;
    split_to    = split;
    join_from   = join;
}

kpair::kpair( kpair &a, 
              raft::kernel_wrapper &b,
              const bool split,
              const bool join ) : kpair( a, *(*b), split, join )
{}


/**
 * opposite of above 
 */
kpair::kpair( raft::kernel &a,
              kpair        &n, 
              const bool split,
              const bool join ) : kpair( a, *(n.in.k) )
{
    head        = this;
    next        = &n;
    split_to    = split;
    join_from   = join;
}

kpair::kpair( raft::kernel_wrapper &a,
              kpair        &n, 
              const bool split,
              const bool join ) : kpair( *(*a), n, split, join )
{}

kpair::kpair( kpair &a,
              kpair &b, 
              const bool split,
              const bool join ) : kpair( *(a.out.k), *(b.in.k) )
{
    head = a.head;
    a.next = this;
    b.head = a.head;
    next = &b;
    split_to  = split;
    join_from = join;
}

kpair::kpair( raft::basekset &a,
              kpair          &b,
              const bool split,
              const bool join )
{
    head      = this;
    next      = &b;
    b.head    = head;

    split_to  = split;
    join_from = join;
    
    in.kset = a.getCopy(); 
    /** we make a copy, don't need to keep a **/
    delete( &a );
    for( const auto &k : (*in.kset) )
    {
        /** 
         * pops the k from the enabled port, so we
         * can only call this exactly once.
         */
        if( k->getEnabledPortCount() == 1 )
        {
            /** push enabled port **/
            src_name.insert( std::make_pair( 
                reinterpret_cast< std::uintptr_t >( k ),
                k->getEnabledPort() ) /** end make pair **/
            );
        }
        else if( k->output.count() > 1 )
        {
            /** 
             * TODO: throw exception since we have more than 
             * one portential port to tie from this source's 
             * output side and we don't know which to use. The
             * user should have specified via the bracket syntax
             * or else made sure the kernel only had one 
             * output port.
             */
            throw PortNotSpecifiedForKSet( "Port must be specified when including a kernel with more than one outgoing port within a kernel set." );
        }
    }
    assert( b.in.k != nullptr );
    out.k = b.in.k;
    const auto dst_temp_name( out.k->getEnabledPort() );

    if( dst_temp_name.length() > 0 )
    {
        /** set false by default **/
        dst_name.insert( std::make_pair( reinterpret_cast< std::uintptr_t >( out.k ), dst_temp_name )  );
    }

    src_out_count = in.kset->getSize();
    dst_in_count  = out.k->input.count();
}

kpair::kpair( raft::basekset &a,
              raft::kernel   &b,
              const bool split,
              const bool join )
{
    /** 
     * this one can either be the first in the list
     * or could be added with some operators to the 
     * source side, either way this one needs to have
     * head and next set to this node
     */
    head      = this;
    next      = this;
    
    split_to  = split;
    join_from = join;
    
    in.kset = a.getCopy(); 
    /** we make a copy, don't need to keep a **/
    delete( &a );
    
    for( const auto &k : (*in.kset) )
    {
        /** 
         * pops the k from the enabled port, so we
         * can only call this exactly once.
         */
        if( k->getEnabledPortCount() == 1 )
        {
            /** push enabled port **/
            src_name.insert( std::make_pair( 
                reinterpret_cast< std::uintptr_t >( k ),
                k->getEnabledPort() ) /** end make pair **/
            );
        }
        else if( k->output.count() > 1 )
        {
            /** 
             * TODO: throw exception since we have more than 
             * one portential port to tie from this source's 
             * output side and we don't know which to use. The
             * user should have specified via the bracket syntax
             * or else made sure the kernel only had one 
             * output port.
             */
            throw PortNotSpecifiedForKSet( "Port must be specified when including a kernel with more than one outgoing port within a kernel set." );
        }
    }
    /** get port names for src_kset **/

    out.k = &b;
    
    const auto dst_temp_name( out.k->getEnabledPort() );

    if( dst_temp_name.length() > 0 )
    {
        dst_name.insert( std::make_pair( reinterpret_cast< std::uintptr_t >( out.k ), dst_temp_name ) );
    }
    /** 
     * re-evaluate if this makes any sense to 
     * count each of these as a separate out
     */
    src_out_count = in.kset->getSize();
    dst_in_count  = out.k->input.count();
}


kpair::kpair( raft::basekset &a,
              raft::basekset &b )
{
    /** 
     * this one can either be the first in the list
     * or could be added with some operators to the 
     * source side, either way this one needs to have
     * head and next set to this node
     */
    head      = this;
    next      = this;
    
    in.kset = a.getCopy(); 
    /** we make a copy, don't need to keep a **/
    delete( &a );
    for( const auto &k : (*in.kset) )
    {
        /** 
         * pops the k from the enabled port, so we
         * can only call this exactly once.
         */
        if( k->getEnabledPortCount() == 1 )
        {
            /** push enabled port **/
            src_name.insert( std::make_pair( 
                reinterpret_cast< std::uintptr_t >( k ),
                k->getEnabledPort() ) /** end make pair **/
            );
        }
        else if( k->output.count() > 1 )
        {
            /** 
             * TODO: throw exception since we have more than 
             * one portential port to tie from this source's 
             * output side and we don't know which to use. The
             * user should have specified via the bracket syntax
             * or else made sure the kernel only had one 
             * output port.
             */
            throw PortNotSpecifiedForKSet( "Port must be specified when including a kernel with more than one outgoing port within a kernel set." );
        }
    }
    
    out.kset = a.getCopy(); 
    /** we make a copy, don't need to keep a **/
    delete( &a );
    /**
     * TODO throw exceptions here to be caught in operator
     * overload syntax if either set has more than one
     * port or more than one enabled port. 
     */
    for( const auto &k : (*out.kset) )
    {
        /** 
         * pops the k from the enabled port, so we
         * can only call this exactly once.
         */
        if( k->getEnabledPortCount() == 1 )
        {
            /** push enabled port **/
            dst_name.insert( std::make_pair( 
                reinterpret_cast< std::uintptr_t >( k ),
                k->getEnabledPort() ) /** end make pair **/
            );
        }
        else if( k->input.count() > 1 )
        {
            /** 
             * TODO: throw exception since we have more than 
             * one portential port to tie from this source's 
             * output side and we don't know which to use. The
             * user should have specified via the bracket syntax
             * or else made sure the kernel only had one 
             * output port.
             */
            throw PortNotSpecifiedForKSet( "Port must be specified when including a kernel with more than one outgoing port within a kernel set." );
        }
    }


}

kpair::kpair( kpair          &a,
              raft::basekset &b,
              const bool split,
              const bool join )
{
    UNUSED( a );
    UNUSED( b );
    UNUSED( split );
    UNUSED( join );
}

kpair::kpair( raft::kernel   &a,
              raft::basekset &b,
              const bool split,
              const bool join )
{
    UNUSED( a );
    UNUSED( b );
    UNUSED( split );
    UNUSED( join );
}




kpair::kpair( raft::kernel &a, raft::kernel &b )
{
    in.k = &a;
    /** see if the source has a name **/
    const auto src_temp_name( a.getEnabledPort() );
    if( src_temp_name.length() > 0 )
    {
        /** set false by default **/
        src_name.insert( std::make_pair( reinterpret_cast< std::uintptr_t >( in.k ), src_temp_name  ) );
    }

    /** see if the destination has a name **/
    out.k = &b;
    const auto dst_temp_name( b.getEnabledPort() );
    if( dst_temp_name.length() > 0 )
    {
        /** set false by default **/
        dst_name.insert( std::make_pair( reinterpret_cast< std::uintptr_t >( out.k ), dst_temp_name ) );
    }

    src_out_count = in.k->output.count();
    dst_in_count  = out.k->input.count();
    head = this;
}


kpair::kpair( raft::kernel &a, 
              raft::kernel_wrapper &b ) : kpair( a, *(*b) )
{
}

kpair::kpair( raft::kernel_wrapper &a, 
              raft::kernel &b ) : kpair( *(*a), b )
{
}

kpair::kpair( raft::kernel_wrapper &a, 
              raft::kernel_wrapper &b ) : kpair( *(*a), *(*b) )
{
}

kpair::~kpair()
{
    /**
     * delete the kset structures, everything else
     * is deleted by the map structure
     */
     if( src_name.size() > 0 )
     {
        delete( in.kset );
        in.kset = nullptr;
     }
     if( dst_name.size() > 0 )
     {
        delete( out.kset );
        out.kset = nullptr;
     }
}


void 
kpair::setOoO() noexcept
{
    (this)->out_of_order = true;
    return;
}

kpair& 
operator >> ( raft::kernel &a, raft::kernel &b )
{
    auto *ptr( new kpair( a, b ) );
    return( *ptr );
}

kpair&
operator >> ( raft::kernel_wrapper &&a, raft::kernel_wrapper &&b )
{
    auto *ptr( new kpair( a, b ) );
    return( *ptr );
}

kpair&
operator >> ( raft::kernel &a, raft::kernel_wrapper &&w )
{
    auto *ptr( new kpair( a, w) );
    return( *ptr );
}



kpair&  
operator >> ( kpair &a, raft::kernel &b )
{
    auto *ptr( new kpair( a, b, false, false ) );
    return( *ptr );
}

kpair&
operator >> ( kpair &a, raft::kernel_wrapper &&w )
{
    auto *ptr( new kpair( a, w, false, false ) );
    return( *ptr );
}

/**
 * >>, we're using the raft::order::spec as a linquistic tool
 * at this point. It's only used for disambiguating functions.
 */
LOoOkpair&
operator >> ( raft::kernel &a, const raft::order::spec &&order )
{
    UNUSED( order );
    auto *ptr( new LOoOkpair( a ) );
    return( *ptr );
}

kpair&
operator >> ( LOoOkpair &a, raft::kernel &b )
{
    auto *ptr( 
        new kpair( a.value, 
                   b, 
                   false, 
                   false ) 
    );
    delete( &a );
    ptr->setOoO();
    return( *ptr );
}


kpair&
operator >> ( LOoOkpair &a, raft::kernel_wrapper &&w )
{
    auto *ptr( 
        new kpair( a.value, w, false, false ) 
    );
    delete( &a );
    ptr->setOoO();
    return( *ptr );
}

/**
 * >>, we're using the raft::order::spec as a linquistic tool
 * at this point. It's only used for disambiguating functions.
 */
ROoOkpair& 
operator >> ( kpair &a, const raft::order::spec &&order )
{
    auto *ptr( new ROoOkpair( a ) );
    UNUSED( order );
    return( *ptr );
}

kpair&
operator >> ( ROoOkpair &a, raft::kernel &b )
{
    auto * ptr(
        new kpair( a.value, b, false, false )
    );
    delete( &a );
    ptr->setOoO();
    return( *ptr );
}

kpair&
operator >> ( ROoOkpair &a, raft::kernel_wrapper &&w )
{
    auto * ptr(
        new kpair( a.value, w, false, false )
    );
    delete( &a );
    ptr->setOoO();
    return( *ptr );
}

kpair&
operator <= ( raft::kernel &a, raft::kernel &b )
{
    auto *ptr( new kpair( a, b, true, false ) );
    return( *ptr );
}

kpair&
operator <= ( raft::kernel_wrapper &&a, raft::kernel_wrapper &&b )
{
    auto *ptr( new kpair( a, b, true, false ) );
    return( *ptr );
}

kpair&
operator <= ( raft::kernel &a, kpair &b )
{
    auto *ptr( new kpair( a, b, true, false ) );
    return( *ptr );
}

kpair&
operator <= ( raft::kernel_wrapper &&w, kpair &b )
{
    auto *ptr( new kpair( w, b, true, false ) );
    return( *ptr );
}

kpair& 
operator <= ( kpair &a, raft::kernel &b )
{
    auto *ptr( new kpair( a, b, true, false ) );
    return( *ptr );
}

kpair&
operator >= ( kpair &a, raft::kernel_wrapper &&w )
{
    auto *ptr( new kpair( a, w, false, true ) );
    return( *ptr );
}

kpair&
operator >= ( kpair &a, raft::kernel &b )
{
    auto *ptr( new kpair( a, b, false, true ) );
    return( *ptr );
}

kpair&
operator >= ( kpair &a, kpair &b )
{
    auto *ptr( new kpair( a, b, false, true ) );
    return(*ptr);
}

kpair& 
operator >= ( raft::kernel &a, kpair &b )
{
    auto *ptr( new kpair( a, b, false, true ) );
    return(*ptr);
}

kpair& 
operator >= ( raft::kernel_wrapper &&w, kpair &b )
{
    auto *ptr( new kpair( w, b, false, true ) );
    return(*ptr);
}

kpair& operator <= ( raft::kernel &a, raft::basekset &&b )
{
    kpair *out( nullptr );
    return(*out );
}

kpair& operator >> ( raft::basekset &&a, raft::kernel &b )
{
    kpair *out( nullptr );
    return(*out);
}

kpair& operator >> ( raft::basekset &&a, raft::basekset &&b )
{
    kpair *out( nullptr );
    return( *out );
}
kpair& operator >= ( raft::basekset &&a, raft::kernel &b )
{
    kpair *out( nullptr );
    return( *out );
}

kpair& operator >= ( raft::basekset &&a, kpair &b )
{
    kpair *out( nullptr );
    return( *out );
}
