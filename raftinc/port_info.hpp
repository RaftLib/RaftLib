/**
 * port_info.hpp -
 * @author: Jonathan Beard
 * @version: Wed Sep  3 20:22:56 2014
 *
 * Copyright 2014 Jonathan Beard
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
#ifndef RAFTPORT_INFO_HPP
#define RAFTPORT_INFO_HPP  1
#include <typeinfo>
#include <typeindex>
#include <string>
#include <map>
#include <functional>
#include <cstddef>
#include <memory>
#include <cassert>

#include "alloc_defs.hpp"
#include "ringbuffertypes.hpp"
#include "port_info_types.hpp"
#include "fifo.hpp"

namespace raft{
    class kernel;
}

struct PortInfo
{
    PortInfo() : type( typeid( ( *this ) ) )
    {
    }

    PortInfo( const std::type_info &the_type ) : type( the_type )
    {
    }

    PortInfo( const std::type_info &the_type,
              void * const ptr,
              const std::size_t nitems,
              const std::size_t start_index ) : type( the_type ),
                                                existing_buffer( ptr ),
                                                nitems( nitems ),
                                                start_index( start_index )
    {
    }

    PortInfo( const PortInfo &other ) : type( other.type )
    {
        fifo_a = other.fifo_a;
        fifo_b = other.fifo_b;
        my_kernel = other.my_kernel;
        my_name = other.my_name;
        other_kernel = other.other_kernel;
        other_name = other.other_name;
        out_of_order = other.out_of_order;
        existing_buffer= other.existing_buffer;
        nitems = other.nitems;
        start_index = other.start_index;
        split_func = other.split_func;
        join_func = other.join_func;
        fixed_buffer_size = other.fixed_buffer_size;
        const_map = other.const_map;
    }

    virtual ~PortInfo() = default;

    /**
     * getFIFO - call this function to get a FIFO, lock free but
     * checks to make sure an update isn't occuring.  The ptr returned
     * will be fine to use even if an update occurs while the ptr
     * is in use since it won't be deleted from the receiving end
     * until the FIFO is fully emptied.
     * @return FIFO*
     */
    FIFO* getFIFO()
    {
        struct
        {
            FIFO *a;
            FIFO *b;
        } copy = { fifo_a, fifo_b };
        /** for most architectures that don't need this,
         * it'll be optimized out after the first iteration **/
        while( copy.a != copy.b )
        {
            copy.a = fifo_a;
            copy.b = fifo_b;
        }
        return( copy.a );
    }

    /**
     * setFIFO - call this funciton to set a FIFO, updates both
     * pointers at the same time as opposed to doing it manually
     * @param   in - valid FIFO*, must not be nullptr
     */
    void setFIFO( FIFO * const in )
    {
        assert( in != nullptr );
        fifo_a = in;
        fifo_b = in;
    }

    FIFO *fifo_a = nullptr;
    FIFO *fifo_b = nullptr;
    /**
     * the type of the port.  regardless of if the buffer itself
     * is impplemented or not.
     */
    std::type_index type;

    /**
     * const_map - stores "builder" objects for each of the
     * currenty implemented ring buffer types so that when
     * the mapper is allocating ring buffers it may allocate
     * one with the proper type.  The first key is self explanatory
     * for the most part, storing the ring buffer type.  The
     * second internal map key is "instrumented" vs. not.
     */
    std::map< Type::RingBufferType,
        std::shared_ptr< instr_map_t > > const_map;


    /**
     * NOTE: These are allocated by the run-time but not
     * destroyed unless they're used...they'll of course
     * be destroyed upon program termination.
     */
    split_factory_t split_func = nullptr;
    join_factory_t join_func = nullptr;

    raft::kernel *my_kernel = nullptr;
    raft::port_key_type my_name = raft::null_port_value;

    raft::kernel *other_kernel = nullptr;
    raft::port_key_type other_name = raft::null_port_value;

    /** runtime settings **/
    bool use_my_allocator = false;
    bool out_of_order = false;
    memory_type mem = heap;
    void *existing_buffer = nullptr;
    std::size_t nitems = 0;
    std::size_t start_index = 0;
    std::size_t fixed_buffer_size = 0;
};
#endif /* END RAFTPORT_INFO_HPP */
