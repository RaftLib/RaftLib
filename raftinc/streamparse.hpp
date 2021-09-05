/**
 * streamparse.hpp - 
 * @author: Jonathan Beard
 * @version: Tue Aug 17 07:23:49 2021
 * 
 * Copyright 2021 Jonathan Beard
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
#ifndef STREAMPARSE_HPP
#define STREAMPARSE_HPP  1

#include "kernel_wrapper.hpp"
#include "parsemap.hpp"

namespace raft
{
    class kernel;
    using parsemap_ptr = std::unique_ptr< raft::parsemap >;
};

template < class T, int N > struct pair_base
{
    constexpr   pair_base( T &t ) : value( t )
    {

    }
    
    virtual     ~pair_base() = default;

    T &value;
};

namespace raft
{

namespace dsl
{

using left_modifier_token  = raft::pair_base< raft::kernel, 0 >; 
using right_modifier_token = raft::pair_base< raft::parsemap_ptr,        0 >;

}

}

raft::parsemap_ptr& operator >> ( raft::kernel &a,  raft::kernel &b  );
raft::parsemap_ptr& operator >> ( raft::kernel_wrapper &&a, raft::kernel_wrapper &&b );
raft::parsemap_ptr& operator >> ( raft::kernel &a, raft::kernel_wrapper &&w );

raft::parsemap_ptr& operator >> ( raft::parsemap_ptr &a, raft::kernel &b );
raft::parsemap_ptr& operator >> ( raft::parsemap_ptr &a, raft::kernel_wrapper &&w );

raft::dsl::left_modifier_token& 
    operator >> ( raft::kernel &a, const raft::order::spec &&order );

raft::parsemap_ptr&     
    operator >> ( raft::dsl::left_modifier_token &a, raft::kernel &b );
raft::parsemap_ptr&     
    operator >> ( raft::dsl::left_modifier_token &a, raft::kernel_wrapper &&w );

right_modifier_token& 
    operator >> ( raft::parsemap_ptr &a, const raft::order::spec &&order );
raft::parsemap_ptr&     operator >> ( right_modifier_token &a, raft::kernel &b );
raft::parsemap_ptr&     operator >> ( right_modifier_token &a, raft::kernel_wrapper &&w );


/**
 * if basekset src/dst are {a,b,c}, {d,e,f}, then 
 * this function links a->d, b->e, c->f. to do so,
 * let's build a zip style operator that'll check
 * to see if src and dst are the same length statically
 * then take an operator. 
 * testcases covered -> ksetContContJoin.cpp
 */
raft::parsemap_ptr&  operator >> ( raft::basekset &src,  raft::basekset &dst  );
raft::parsemap_ptr&  operator >> ( raft::basekset &&src, raft::basekset &&dst );

raft::parsemap_ptr& operator <= ( raft::kernel &a, raft::kernel  &b );
raft::parsemap_ptr& operator <= ( raft::kernel_wrapper &&a, raft::kernel_wrapper &&b );
raft::parsemap_ptr& operator <= ( raft::kernel &a,  raft::parsemap_ptr &b );
raft::parsemap_ptr& operator <= ( raft::kernel_wrapper &&w, raft::parsemap_ptr &b );

raft::parsemap_ptr& operator >= ( raft::kernel &a, raft::kernel &b );
raft::parsemap_ptr& operator >= ( raft::kernel_wrapper &&a, raft::kernel_wrapper &&b );
raft::parsemap_ptr& operator >= ( raft::parsemap_ptr &a, raft::kernel &b );
raft::parsemap_ptr& operator >= ( raft::parsemap_ptr &a, raft::kernel_wrapper &&w );
raft::parsemap_ptr& operator >= ( raft::parsemap_ptr &a, raft::parsemap_ptr &b );

raft::parsemap_ptr& operator >= ( raft::kernel &a, raft::parsemap_ptr &b );
raft::parsemap_ptr& operator >= ( raft::kernel_wrapper &&w, raft::parsemap_ptr &b );

#endif /* END STREAMPARSE_HPP */
