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
#include "kset.tcc"
#include "defs.hpp"

namespace raft
{
    class kernel;
    class kernel_wrapper;
    class map;
}

class kpair;

template < class T, int N > struct PairBase
{
    constexpr PairBase( T &t ) : value( t ){};
    virtual ~PairBase() = default;

    T &value;
};


using LOoOkpair = PairBase< raft::kernel, 0 >; 
using ROoOkpair = PairBase< kpair,        0 >;


class kpair
{
public:
    kpair( raft::kernel &a, 
           raft::kernel &b,
           const bool split,
           const bool join );
    
    kpair( raft::kernel &a, 
           raft::kernel_wrapper &b,
           const bool split,
           const bool join );
    
    kpair( raft::kernel_wrapper &a, 
           raft::kernel &b,
           const bool split,
           const bool join );
    
    kpair( raft::kernel_wrapper &a, 
           raft::kernel_wrapper &b,
           const bool split,
           const bool join );

    kpair( kpair &a,
           raft::kernel  &b,
           const bool split,
           const bool join );
    
    kpair( kpair &a,
           raft::kernel_wrapper  &b,
           const bool split,
           const bool join );

    kpair( raft::kernel &a,
           kpair        &n,
           const bool   split,
           const bool   join );
    
    kpair( raft::kernel_wrapper &a,
           kpair        &n,
           const bool   split,
           const bool   join );

    kpair( kpair &a,
           kpair &b,
           const bool split,
           const bool join );

    kpair( raft::kernel &a, raft::kernel &b );
    
    kpair( raft::kernel &a, 
           raft::kernel_wrapper &b );
    
    kpair( raft::kernel_wrapper &a, 
           raft::kernel  &b );

    kpair( raft::kernel_wrapper &a, 
           raft::kernel_wrapper &b );
    
    void setOoO() noexcept;

protected:
    kpair        *next          = nullptr;
    kpair        *head          = nullptr;
    raft::kernel *src           = nullptr;
    bool          has_src_name  = false;
    raft::port_key_type src_name = raft::null_port_value;
    raft::kernel *dst           = nullptr;
    bool          has_dst_name  = false;
    raft::port_key_type dst_name = raft::null_port_value;
    
    std::size_t   buffer_size   = 0;
    
    bool          split_to      = false;
    core_id_t     src_out_count = 0;
    bool          join_from     = false;
    core_id_t     dst_in_count  = 0;

    bool          out_of_order  = false;
    friend class raft::map;
};



kpair& operator >> ( raft::kernel &a,  raft::kernel &b  );
kpair& operator >> ( raft::kernel_wrapper &&a, raft::kernel_wrapper &&b );
kpair& operator >> ( raft::kernel &a, raft::kernel_wrapper &&w );

kpair& operator >> ( kpair &a, raft::kernel &b );
kpair& operator >> ( kpair &a, raft::kernel_wrapper &&w );

LOoOkpair& operator >> ( raft::kernel &a, const raft::order::spec &&order );
kpair&     operator >> ( LOoOkpair &a, raft::kernel &b );
kpair&     operator >> ( LOoOkpair &a, raft::kernel_wrapper &&w );

ROoOkpair& operator >> ( kpair &a, const raft::order::spec &&order );
kpair&     operator >> ( ROoOkpair &a, raft::kernel &b );
kpair&     operator >> ( ROoOkpair &a, raft::kernel_wrapper &&w );


kpair& operator <= ( raft::kernel &a, raft::kernel  &b );
kpair& operator <= ( raft::kernel_wrapper &&a, raft::kernel_wrapper &&b );
kpair& operator <= ( raft::kernel &a,  kpair &b );
kpair& operator <= ( raft::kernel_wrapper &&w, kpair &b );

kpair& operator >= ( raft::kernel &a, raft::kernel &b );
kpair& operator >= ( raft::kernel_wrapper &&a, raft::kernel_wrapper &&b );
kpair& operator >= ( kpair &a, raft::kernel &b );
kpair& operator >= ( kpair &a, raft::kernel_wrapper &&w );
kpair& operator >= ( kpair &a, kpair &b );

kpair& operator >= ( raft::kernel &a, kpair &b );
kpair& operator >= ( raft::kernel_wrapper &&w, kpair &b );


#endif /* END RAFTKPAIR_HPP */
