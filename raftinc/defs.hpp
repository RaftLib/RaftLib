/**
 * defs.hpp - 
 * @author: Jonathan Beard
 * @version: Sun Feb  7 05:46:48 2016
 * 
 * Copyright 2016 Jonathan Beard
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
#ifndef RAFTDEFS_HPP
#define RAFTDEFS_HPP  1
#include <set>
#include <map>
#include <functional>
#include <type_traits>
#include <cstdint>
#include <vector>
#include <utility>
#include <climits>
#include <bitset>
#include <memory>
#include <string>

namespace raft
{
    /** predeclare raft::kernel for kernel_list_t below **/
    class kernel;
    /** also parsemap_ptr smart ptr obj used in map and streamparse **/
    class parsemap;
    using port_key_type = std::string;
    using parsemap_ptr = std::shared_ptr< raft::parsemap >;
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

namespace raft
{
    using byte_t = std::uint8_t;
    const static std::uint8_t bits_per_byte = CHAR_BIT;
}

/** system config defs **/
namespace raft
{

/** 
 * type for stream manipulation, currently
 * this means that there are 64 available
 * modifiers. we can always make this a 
 * longer array at a later point if we 
 * for some reason need more.
 */
using manip_vec_t = std::uint64_t;

/**
 * NOTE: in addition to registering the int type here
 * for parsing, if a developer wants to add another
 * state, there must a corresponding static function
 * defined in the mapbase file so that the 
 * parse function pointers can be called for the 
 * appropriate int below and perform the correct
 * setting actions for each kernel
 */

/** raft::parallel **/
namespace parallel
{

enum type : manip_vec_t { 
    system = 0  /** do whatever the runtime wants, I don't care  **/,
    thread      /** specify a thread for each kernel **/, 
    pool        /** thread pool, one kernel thread per core, many kernels in each **/, 
    process     /** open a new process from this point **/,
    PARALLEL_N };    
} /** end namespace parallel **/ 

/** raft::vm **/
namespace vm
{

enum type : manip_vec_t { 
    flat = parallel::PARALLEL_N       /** not yet implemented, likely using segment  **/, 
    standard                          /** threads share VM space, processes have sep **/, 
    partition                         /** partition graph at this point into a 
                                        * new VM space, platform dependent **/,
    VM_N
}; 
} /** end namespace vm **/

namespace order
{
enum spec : std::uint8_t { 
    in = vm::VM_N, 
    out, 
    ORDER_N 
};
} /** end namespace order **/


/**
 * these are to enable the sub-kernel behavior where 
 * the kernel specifies that all ports must be active 
 * before firing the kernel...by "active" we mean that
 * all ports have some data.
 */
enum schedule_behavior : std::uint8_t { any_port = 0,
                                        all_port = 1 };

} /** end namespace raft **/

#endif /* END RAFTDEFS_HPP */
