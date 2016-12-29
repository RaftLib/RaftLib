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
#include "raftmanip.hpp"


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

template < class T, class K, int N > struct ParaPair : PairBase< T, N >
{
    constexpr ParaPair( T &t, const K sp ) : PairBase< T, N >( t ),
                                             sp( sp ){};
    const K sp;
};


using LParaPair = ParaPair< raft::kernel,raft::parallel::type, 1 >;
using RParaPair = ParaPair< kpair, raft::parallel::type, 1 >;

using LOoOkpair = PairBase< raft::kernel, 2 >; 
using ROoOkpair = PairBase< kpair, 2 >;


class kpair
{
public:
    kpair( raft::kernel &src, 
           raft::kernel &dst,
           const bool split,
           const bool join );
    
    kpair( raft::kernel &src, 
           raft::kernel_wrapper &dst,
           const bool split,
           const bool join );
    
    kpair( raft::kernel_wrapper &src, 
           raft::kernel &dst,
           const bool split,
           const bool join );
    
    kpair( raft::kernel_wrapper &src, 
           raft::kernel_wrapper &dst,
           const bool split,
           const bool join );

    kpair( kpair &src,
           raft::kernel  &dst,
           const bool split,
           const bool join );
    
    kpair( kpair &src,
           raft::kernel_wrapper  &dst,
           const bool split,
           const bool join );

    kpair( raft::kernel &src,
           kpair        &dst,
           const bool   split,
           const bool   join );
    
    kpair( raft::kernel_wrapper &src,
           kpair        &dst,
           const bool   split,
           const bool   join );

    kpair( kpair &src,
           kpair &dst,
           const bool split,
           const bool join );

    kpair( raft::basekset &src,
           kpair &dst,
           const bool split,
           const bool join );
    
    kpair( raft::basekset   &src,
           raft::kernel     &dst,
           const bool       split,
           const bool       join );

    kpair( raft::basekset &src,
           raft::basekset &dst );

    kpair( kpair &src,
           raft::basekset &dst,
           const bool split,
           const bool join );

    kpair( raft::kernel     &src,
           raft::basekset   &dst,
           const bool       split,
           const bool       join );

    kpair( raft::kernel &src, raft::kernel &dst );
    
    kpair( raft::kernel &src, 
           raft::kernel_wrapper &dst );
    
    kpair( raft::kernel_wrapper &src, 
           raft::kernel  &dst );

    kpair( raft::kernel_wrapper &src, 
           raft::kernel_wrapper &dst );
    
    void setOoO() noexcept;

    virtual ~kpair();

protected:
    /** 
     * Head and next pointers so we can make a linked list
     * of the chain created by the >> from the user. These
     * will be used to create the graph structure by the
     * map class.
     */
    kpair                       *next          = nullptr;
    kpair                       *head          = nullptr;
    
    /** 
     * I didn't feel like using 8-bytes more than I had to
     * these structure pointers are mutually exlusive, 
     * meaning we're either using one or the other and
     * its totall apparent within the context of use which
     * is in fact valid - jcb.
     */
    union
    {
        raft::kernel    *k      = nullptr;
        raft::basekset  *kset;
    } in;

    /**
     * map to accomodate kset structures, it is indexed
     * on the memory location of the kernel that the
     * name is valid to, i.e. a[ " out" ] is referenced
     * on the kernel memory location (&a) as a ptr int
     */
    std::map< std::uintptr_t, std::string >     src_name;
    
    /** 
     * I didn't feel like using 8-bytes more than I had to
     * these structure pointers are mutually exlusive, 
     * meaning we're either using one or the other and
     * its totall apparent within the context of use which
     * is in fact valid - jcb.
     */
    union
    {
        raft::kernel    *k      = nullptr;
        raft::basekset  *kset;
    } out;
    /**
     * map to accomodate kset structures, it is indexed
     * on the memory location of the kernel that the
     * name is valid to, i.e. a[ " out" ] is referenced
     * on the kernel memory location (&a) as a ptr int
     */
    std::map< std::uintptr_t, std::string >     dst_name;

    
    bool             split_to      = false;
    
    core_id_t        src_out_count = 0;
    
    bool             join_from     = false;
    
    core_id_t        dst_in_count  = 0;

    bool             out_of_order  = false;
    
    raft::parallel::type    context_type  = raft::parallel::system;  
    
    friend class raft::map;
};
#endif /* END _KPAIR_HPP_ */
