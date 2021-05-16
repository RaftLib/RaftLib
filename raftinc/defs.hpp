/**
 * defs.hpp - 
 * @author: Jonathan Beard
 * @version: Sun Feb  7 05:46:48 2016
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
#include <limits>





#if (! defined STRING_NAMES) || (STRING_NAMES == 0)
#include "hh.hpp"
#endif

namespace raft
{
    /** predeclare raft::kernel for kernel_list_t below **/
    class kernel;
    /** also parsemap_ptr smart ptr obj used in map and streamparse **/
    class parsemap;
/** use this to turn on string names vs. off **/
/**
 * Notes: For port_key_type, there's the original string version
 * which turns out to be rather cumbersome for very small kernels,
 * and then there's the version that does hashing of strings then 
 * uses the 64b hash of that string. There is a tiny change of port
 * name collision with the 64b version, however, it's incredibly 
 * unlikely that this would occur. 
 */
#ifdef STRING_NAMES
    using port_key_type = std::string;
    const static raft::port_key_type null_port_value = "";
#else
    /**
     * set max length of the string for the fixed length representation
     * of the port name, will be used for debug only, doesn't really
     * constrain the length used by programmers when typing port names. 
     */
    const static std::uint32_t  port_name_max_length = 64;
    /**
     * define the type of the port key, this is the value typed in by the
     * programmer to name ports. From the programmer perspective it'll look
     * like a string. 
     */
    using port_key_type = highway_hash::hash_t::val_type;
    
    template < std::size_t N > using name_struct_t = highway_hash::data_t< N >;
    /**
     * just like the string, we need a value for uninitialized port
     * types. 
     */
    const static raft::port_key_type null_port_value = 
        std::numeric_limits< raft::port_key_type >::max();
    /**
     * fixed length name representation containing both the hash value
     * and the string name of the hash (although it is truncated to the
     * max selected length above. 
     */
    using port_key_name_t = highway_hash::data_fixed_t< raft::port_name_max_length >;
#endif
    using parsemap_ptr = std::shared_ptr< raft::parsemap >;
} /** end namespace raft **/

#ifndef STRING_NAMES
    /**
     * use this to get a constexpr 64b unsigned hash
     * of a string. Must compile with C++20 for this to
     * work given it requires return type template type
     * deduction for user-defined string literals. 
     * e.g. "foobar"_port, hashes the string at compile
     * time. Currently only g++ has this capability, maybe
     * the latest head of clang, apple clang does not. 
     * The return type is a struct with the string, 
     * the length, and the hash value. e.g.
     * auto data( "foobar"_port); then the field data.val
     * contains your hash. 
     */
    template < raft::name_struct_t port_name >
    static
    constexpr
    auto
    operator""_port() 
    {
        return( port_name );
    }
#endif

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


namespace order
{
enum spec : std::uint8_t { 
    in = 0, 
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
