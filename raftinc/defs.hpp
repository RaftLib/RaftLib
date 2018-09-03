/**
 * defs.hpp - 
 * @author: Jonathan Beard
 * @version: Mon Sep  3 16:20:48 2018
 * 
 * Copyright 2018 Jonathan Beard
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
#ifndef _DEFS_HPP_
#define _DEFS_HPP_  1
#include <set>
#include <map>
#include <functional>
#include <type_traits>
#include <cstdint>
#include <vector>
#include <utility>


/** predeclare raft::kernel for kernel_list_t below **/
namespace raft
{
    class kernel;
} /** end namespace raft **/

template < typename T > 
    using set_t = std::set< T >;

using ptr_set_t = set_t< std::uintptr_t >;
using recyclefunc_t = std::function< void( void * ) >;
using ptr_map_t = std::map< std::uintptr_t, recyclefunc_t >;
using ptr_t = std::uintptr_t;

using core_id_t = std::int64_t;

/** type for edge weights when giving to Scotch partitioner **/
using weight_t  = std::int32_t;
/** type for edge id, largely used by graph.tcc and partition_scotch.hpp **/
using edge_id_t = std::int32_t;

#ifndef UNUSED 
#ifdef __clang__
#define UNUSED( x ) (void)(x)
#else
#define UNUSED( x )[&x]{}()
#endif
//FIXME need to double check to see IF THIS WORKS ON MSVC
#endif

/** type for return from += you'll get an iterator to one of these **/
using kernel_list_t  = std::vector< std::reference_wrapper< raft::kernel > >;
using kernel_it_pair = std::pair< 
                                    typename kernel_list_t::const_iterator,
                                    typename kernel_list_t::const_iterator
                                >;

using byte_t = std::uint8_t;


#if (defined __linux) || (defined __APPLE__ )

#define R_LIKELY( var ) __builtin_expect( var, 1 )
#define R_UNLIKELY( var ) __builtin_expect( var, 0 )

#else

#define R_LIKELY( var ) var
#define R_UNLIKELY( var ) var

#endif

namespace raft
{
enum schedule_behavior : std::uint8_t { any_port = 0,
                                        all_port = 1 };
}
#endif /* END _DEFS_HPP_ */
