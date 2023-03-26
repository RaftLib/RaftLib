/**
 * kpair.hpp -
 * @author: Jonathan Beard
 * @version: 25 May 2020
 *
 * Copyright 2020 Jonathan Beard
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef RAFTKPAIR_HPP
#define RAFTKPAIR_HPP  1

#include <string>
#include <stack>
#include "kset.tcc"
#include "defs.hpp"

namespace raft
{
    class kernel;
}

class kpair;
struct KernelPortMeta;

template < class T, int N > struct PairBase
{
    constexpr PairBase( T &t ) : value( t ){};
    virtual ~PairBase() = default;

    T &value;
};


using LOoOkpair = PairBase< raft::kernel,   0 >;
using ROoOkpair = PairBase< kpair,          0 >;
using MOoOkpair = PairBase< KernelPortMeta, 0 >;

struct KernelPortMeta
{
    raft::kernel *kernel;
    std::stack<raft::port_key_type> port_names;
    core_id_t out_ports_count, in_ports_count;
    KernelPortMeta ( raft::kernel *k,
                     raft::port_key_type name = raft::null_port_value,
                     core_id_t out_count = 1,
                     core_id_t in_count = 1 ) :
        kernel( k ), out_ports_count( out_count ), in_ports_count( in_count )
    {
        port_names.push(name);
    }
    KernelPortMeta ( const KernelPortMeta &rhs )
    {
        kernel = rhs.kernel;
        port_names = rhs.port_names;
        out_ports_count = rhs.out_ports_count;
        in_ports_count = rhs.in_ports_count;
    }
    KernelPortMeta operator []( raft::port_key_type name )
    {
        port_names.push(name);
        return *this;
    }

    /**
     * >>, we're using the raft::order::spec as a linquistic tool
     * at this point. It's only used for disambiguating functions.
     */
    MOoOkpair&
    operator >> ( const raft::order::spec &&order )
    {
        UNUSED( order );
        auto *ptr( new MOoOkpair( *this ) );
        return( *ptr );
    }
};

class kpair
{
public:
    kpair( raft::kernel *a,
           raft::kernel *b,
           const bool split = false,
           const bool join = false ) :
        kpair( new KernelPortMeta(a), new KernelPortMeta(b) )
    {
        split_to = split;
        join_from = join;
    }

    kpair( raft::kernel *a,
           kpair &b,
           const bool split = false,
           const bool join = false ) :
        kpair( new KernelPortMeta(a), b.src_meta )
    {
        head = this;
        next = &b;
        b.head = this;
        split_to = split;
        join_from = join;
    }

    kpair( raft::kernel *a,
           KernelPortMeta *b,
           const bool split = false,
           const bool join = false ) :
        kpair( new KernelPortMeta( a ), b )
    {
        split_to = split;
        join_from = join;
    }

    kpair( kpair &a,
           raft::kernel *b,
           const bool split = false,
           const bool join = false ) :
        kpair( a.dst_meta, new KernelPortMeta(b) )
    {
        head = a.head;
        a.next = this;
        split_to = split;
        join_from = join;
    }

    kpair( kpair &a,
           kpair &b,
           const bool split = false,
           const bool join = false ) :
        kpair( a.dst_meta, b.src_meta )
    {
        head = a.head;
        a.next = this;
        b.head = a.head;
        next = &b;
        split_to = split;
        join_from = join;
    }

    kpair( kpair &a,
           KernelPortMeta *b,
           const bool split = false,
           const bool join = false ) :
        kpair( a.dst_meta, b )
    {
        head = a.head;
        a.next = this;
        split_to = split;
        join_from = join;
    }

    kpair( KernelPortMeta *a,
           raft::kernel *b,
           const bool split = false,
           const bool join = false ) :
        kpair( a, new KernelPortMeta(b) )
    {
        split_to = split;
        join_from = join;
    }

    kpair( KernelPortMeta *a,
           kpair &b,
           const bool split = false,
           const bool join = false ) :
        kpair( a, b.src_meta )
    {
        head = this;
        next = &b;
        b.head = this;
        split_to = split;
        join_from = join;
    }

    kpair( KernelPortMeta *a,
           KernelPortMeta *b,
           const bool split = false,
           const bool join = false )
    {
        src_meta = a;
        src = a->kernel;
        if ( 0 == a->port_names.size() )
        {
            src_name = raft::null_port_value;
        } else {
            src_name = a->port_names.top();
            a->port_names.pop();
        }
        if( src_name != raft::null_port_value )
        {
            /** set false by default **/
            has_src_name = true;
        }
        dst_meta = b;
        dst = b->kernel;
        if ( 0 == b->port_names.size() )
        {
            dst_name = raft::null_port_value;
        } else {
            dst_name = b->port_names.top();
            b->port_names.pop();
        }
        if( dst_name != raft::null_port_value )
        {
            /** set false by default **/
            has_dst_name = true;
        }
        src_out_count = a->out_ports_count;
        dst_in_count = b->in_ports_count;
        head = this;

        split_to = split;
        join_from = join;
    }

    kpair&
    operator >> ( raft::kernel &rhs )
    {
        auto *ptr( new kpair( *this, &rhs, false, false ) );
        return( *ptr );
    }

    kpair&
    operator >> ( kpair &rhs )
    {
        auto *ptr( new kpair( *this, rhs, false, false ) );
        return( *ptr );
    }

    kpair&
    operator >> ( const KernelPortMeta &rhs )
    {
        KernelPortMeta *meta_ptr = new KernelPortMeta(rhs);
        auto *ptr( new kpair( *this, meta_ptr, false, false ) );
        return( *ptr );
    }

    kpair&
    operator >> ( KernelPortMeta *rhs )
    {
        auto *ptr( new kpair( *this, rhs, false, false ) );
        return( *ptr );
    }

    kpair&
    operator >= ( raft::kernel &rhs )
    {
        auto *ptr( new kpair( *this, &rhs, false, true ) );
        return( *ptr );
    }

    kpair&
    operator >= ( kpair &rhs )
    {
        auto *ptr( new kpair( *this, rhs, false, true ) );
        return( rhs );
    }

    kpair&
    operator >= ( const KernelPortMeta &rhs )
    {
        KernelPortMeta *meta_ptr = new KernelPortMeta(rhs);
        auto *ptr( new kpair( *this, meta_ptr, false, true ) );
        return( *ptr );
    }

    kpair&
    operator >= ( KernelPortMeta *rhs )
    {
        auto *ptr( new kpair( *this, rhs, false, true ) );
        return( *ptr );
    }

    kpair&
    operator <= ( raft::kernel &rhs )
    {
        auto *ptr( new kpair( *this, &rhs, true, false ) );
        return( *ptr );
    }

    kpair&
    operator <= ( kpair &rhs )
    {
        auto *ptr( new kpair( *this, rhs, true, false ) );
        return( rhs );
    }

    kpair&
    operator <= ( const KernelPortMeta &rhs )
    {
        KernelPortMeta *meta_ptr = new KernelPortMeta(rhs);
        auto *ptr( new kpair( *this, meta_ptr, true, false ) );
        return( *ptr );
    }

    kpair&
    operator <= ( KernelPortMeta *rhs )
    {
        auto *ptr( new kpair( *this, rhs, true, false ) );
        return( *ptr );
    }

    /**
     * >>, we're using the raft::order::spec as a linquistic tool
     * at this point. It's only used for disambiguating functions.
     */
    ROoOkpair&
    operator >> ( const raft::order::spec &&order )
    {
        auto *ptr( new ROoOkpair( *this ) );
        UNUSED( order );
        return( *ptr );
    }

    void setOoO() noexcept
    {
        (this)->out_of_order = true;
        return;
    }

protected:
    kpair *next = nullptr;
    kpair *head = nullptr;
    KernelPortMeta *src_meta = nullptr;
    KernelPortMeta *dst_meta = nullptr;
    raft::kernel *src = nullptr;
    raft::kernel *dst = nullptr;
    bool has_src_name = false;
    bool has_dst_name = false;
    raft::port_key_type src_name = raft::null_port_value;
    raft::port_key_type dst_name = raft::null_port_value;

    std::size_t buffer_size = 0;

    bool split_to = false;
    bool join_from = false;
    core_id_t src_out_count = 0;
    core_id_t dst_in_count = 0;

    bool out_of_order = false;
    friend class raft::map;
};

template < class T, int N >
kpair&
operator >> ( PairBase < T, N > &a, raft::kernel *b )
{
    /* this is just to make the compiler happy, would not really instantiate */
    auto *dummy_ptr(
        new kpair( b,
                   b,
                   false,
                   false )
    );
    return( *dummy_ptr );
}

template < class T, int N >
kpair&
operator >> ( PairBase < T, N > &a, raft::kernel &b )
{
    /* this is just to make the compiler happy, would not really instantiate */
    auto *dummy_ptr(
        new kpair( &b,
                   &b,
                   false,
                   false )
    );
    return( *dummy_ptr );
}

template < class T, int N >
kpair&
operator >> ( PairBase < T, N > &a, KernelPortMeta b )
{
    /* this is just to make the compiler happy, would not really instantiate */
    auto *dummy_ptr(
        new kpair( &b,
                   &b,
                   false,
                   false )
    );
    return( *dummy_ptr );
}

template < int N >
kpair&
operator >> ( PairBase < raft::kernel, N > &a, raft::kernel *b )
{
    auto *ptr(
        new kpair( &a.value,
                   b,
                   false,
                   false )
    );
    delete( &a );
    ptr->setOoO();
    return( *ptr );
}

template < int N >
kpair&
operator >> ( PairBase < kpair, N > &a, raft::kernel *b )
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

template < int N >
kpair&
operator >> ( PairBase < KernelPortMeta, N > &a, raft::kernel *b )
{
    auto *ptr(
        new kpair( &a.value,
                   b,
                   false,
                   false )
    );
    //delete( &a );
    //ptr->setOoO();
    return( *ptr );
}

template < int N >
kpair&
operator >> ( PairBase < raft::kernel, N > &a, raft::kernel &b )
{
    auto *ptr(
        new kpair( &a.value,
                   &b,
                   false,
                   false )
    );
    delete( &a );
    ptr->setOoO();
    return( *ptr );
}

template < int N >
kpair&
operator >> ( PairBase < kpair, N > &a, raft::kernel &b )
{
    auto *ptr(
        new kpair( a.value,
                   &b,
                   false,
                   false )
    );
    delete( &a );
    ptr->setOoO();
    return( *ptr );
}

template < int N >
kpair&
operator >> ( PairBase < KernelPortMeta, N > &a, raft::kernel &b )
{
    auto *ptr(
        new kpair( &a.value,
                   &b,
                   false,
                   false )
    );
    //delete( &a );
    //ptr->setOoO();
    return( *ptr );
}

template < int N >
kpair&
operator >> ( PairBase < raft::kernel, N > &a, KernelPortMeta b )
{
    auto *ptr(
        new kpair( &a.value,
                   new KernelPortMeta( b ),
                   false,
                   false )
    );
    delete( &a );
    ptr->setOoO();
    return( *ptr );
}

template < int N >
kpair&
operator >> ( PairBase < kpair, N > &a, KernelPortMeta b )
{
    auto *ptr(
        new kpair( a.value,
                   new KernelPortMeta( b ),
                   false,
                   false )
    );
    delete( &a );
    ptr->setOoO();
    return( *ptr );
}

template < int N >
kpair&
operator >> ( PairBase < KernelPortMeta, N > &a, KernelPortMeta b )
{
    auto *ptr(
        new kpair( &a.value,
                   new KernelPortMeta( b ),
                   false,
                   false )
    );
    //delete( &a );
    //ptr->setOoO();
    return( *ptr );
}

#endif /* END RAFTKPAIR_HPP */
