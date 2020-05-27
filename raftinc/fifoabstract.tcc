/**
 * fifoabstract.tcc - encapsulate some of the common components
 * of a FIFO implementation.  
 *
 * @author: Jonathan Beard
 * @version: Sun Sep  7 20:36:12 2014
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
#ifndef RAFTFIFOABSTRACT_TCC
#define RAFTFIFOABSTRACT_TCC  1
#include "ringbuffertypes.hpp"
#include "bufferdata.tcc"
#include "blocked.hpp"
#include "fifo.hpp"
#include "datamanager.tcc"
#include "defs.hpp"
#include "internaldefs.hpp"

template < class T, Type::RingBufferType type > 
   class FIFOAbstract : public FIFO
{
public:
   FIFOAbstract() : FIFO(){}

protected:

    inline void init() noexcept
    {
        auto * const buffer( datamanager.get() );
        assert( buffer != nullptr );
        producer_data.write_stats = &buffer->write_stats;
        consumer_data.read_stats  = &buffer->read_stats;
    }

    struct ALIGN( L1D_CACHE_LINE_SIZE ) {
        volatile bool            allocate_called = false;
        Blocked::value_type      n_allocated     = 1;
        /**
         * these pointers are set by the scheduler which 
         * calls the garbage collection function. these
         * two capture the addresses of output pointers
         */
        ptr_set_t                   *out         = nullptr;
        ptr_set_t                   *out_peek    = nullptr;
        /** 
         * this is set via init callback on fifo construction
         * this prevents the re-calculating of the address
         * over and over....pointer chasing is very bad
         * for cache performance.
         */
        Blocked                     *write_stats = nullptr;
    } producer_data;
   
   
    struct  ALIGN( L1D_CACHE_LINE_SIZE ) {
        /**
         * these pointers are set by the scheduler which 
         * calls the garbage collection function. these
         * two capture the addresses of output pointers
         */
        ptr_map_t                   *in         = nullptr;
        ptr_set_t                   *in_peek    = nullptr;
        Blocked                     *read_stats = nullptr;
    } consumer_data;
    
    /** 
     * upgraded the *data structure to be a DataManager
     * object to enable easier and more intuitive dynamic
     * lock free buffer resizing and re-alignment.
     */
    DataManager< T, type >       datamanager;
};
#endif /* END RAFTFIFOABSTRACT_TCC */
