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
#ifndef _FIFOABSTRACT_TCC_
#define _FIFOABSTRACT_TCC_  1
#include "ringbuffertypes.hpp"
#include "bufferdata.tcc"
#include "blocked.hpp"
#include "fifo.hpp"
#include "datamanager.tcc"

template < class T, Type::RingBufferType type > 
   class FIFOAbstract : public FIFO
{
public:
   FIFOAbstract() : FIFO(){}

protected:
    struct{
        volatile bool            allocate_called = false;
        Blocked::value_type      n_allocated     = 1;
        /**
         * these pointers are set by the scheduler which 
         * calls the garbage collection function. these
         * two capture the addresses of output pointers
         */
        ptr_set_t                   *out = nullptr;
        ptr_set_t                   *out_peek = nullptr;
    }producer_data;
   
   
    struct{
        /**
         * these pointers are set by the scheduler which 
         * calls the garbage collection function. these
         * two capture the addresses of output pointers
         */
        ptr_map_t                   *in = nullptr;
        ptr_set_t                   *in_peek  = nullptr;
    }consumer_data;
    
    /** 
     * upgraded the *data structure to be a DataManager
     * object to enable easier and more intuitive dynamic
     * lock free buffer resizing and re-alignment.
     */
    DataManager< T, type >       datamanager;
};
#endif /* END _FIFOABSTRACT_TCC_ */
