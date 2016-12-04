/**
 * kpair.hpp - 
 * @author: Jonathan Beard
 * @version: Wed Dec  9 11:36:08 2015
 * 
 * Copyright 2015 Jonathan Beard
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
#ifndef _KPAIR_HPP_
#define _KPAIR_HPP_  1

#include <string>
#include <vector>
#include "kset.tcc"
#include "portorder.hpp"
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

    kpair( raft::basekset &a,
           kpair &b,
           const bool split,
           const bool join );
    
    kpair( raft::basekset   &a,
           raft::kernel     &b,
           const bool       split,
           const bool       join );

    kpair( raft::basekset &a,
           raft::basekset &b );

    kpair( kpair &a,
           raft::basekset &b,
           const bool split,
           const bool join );

    kpair( raft::kernel     &a,
           raft::basekset   &b,
           const bool       split,
           const bool       join );

    kpair( raft::kernel &a, raft::kernel &b );
    
    kpair( raft::kernel &a, 
           raft::kernel_wrapper &b );
    
    kpair( raft::kernel_wrapper &a, 
           raft::kernel  &b );

    kpair( raft::kernel_wrapper &a, 
           raft::kernel_wrapper &b );
    
    void setOoO() noexcept;

    virtual ~kpair()
    {
        /**
         * delete the kset structures, everything else
         * is deleted by the map structure
         */
         delete( src_kset );
         src_kset = nullptr;
         delete( dst_kset );
         dst_kset = nullptr;
    }

protected:
    kpair                       *next          = nullptr;
    kpair                       *head          = nullptr;
    raft::kernel                *src           = nullptr;
    bool                         has_src_name  = false;
    /** vector to accomodate kset structures **/
    std::vector< std::string >   src_name;
    raft::kernel                *dst           = nullptr;
    bool                         has_dst_name  = false;
    /** vector to accomodate kset structures **/
    std::vector< std::string >   dst_name;
    /** created via getCopy so these need to be deleted **/
    raft::basekset  *src_kset     = nullptr;
    raft::basekset  *dst_kset     = nullptr;
         
    
    bool             split_to      = false;
    core_id_t        src_out_count = 0;
    bool             join_from     = false;
    core_id_t        dst_in_count  = 0;

    bool             out_of_order  = false;
    raft::parallel::type    context_type  = raft::parallel::system;  
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
kpair& operator <= ( kpair &a, raft::kernel &b );


kpair& operator >= ( kpair &a, raft::kernel &b );
kpair& operator >= ( kpair &a, raft::kernel_wrapper &&w );
kpair& operator >= ( kpair &a, kpair &b );

kpair& operator >= ( raft::kernel &a, kpair &b );
kpair& operator >= ( raft::kernel_wrapper &&w, kpair &b );


kpair& operator <= ( raft::kernel &a, raft::basekset &&b );
kpair& operator >> ( RParaPair &a, const raft::parallel::type &&type );

kpair& operator >> ( raft::basekset &&a, raft::kernel &b );
kpair& operator >> ( raft::basekset &&a, raft::basekset &&b );
/**
 * this case looks like this:
 * m += raft::kset( g0, g1, g2 ) >= j;
 */
kpair& operator >= ( raft::basekset &&a, raft::kernel &b );
/**
 * this case looks like this:
 * m += raft::kset( g0, g1, g2 ) >= j >> print;
 * due to >> operator precendent in c++
 */
kpair& operator >= ( raft::basekset &&a, kpair &b ); 
#endif /* END _KPAIR_HPP_ */
