/**
 * allocate.cpp -
 * @author: Jonathan Beard
 * @version: Tue Sep 16 20:20:06 2014
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
#include <cassert>
#include <thread>

#include "fifo.hpp"

#include "allocate.hpp"
#include "port_info.hpp"
#include "map.hpp"
#include "portexception.hpp"

Allocate::Allocate( raft::map &map, volatile bool &exit_alloc ) :
   source_kernels( map.source_kernels ),
   all_kernels(    map.all_kernels ),
   exit_alloc( exit_alloc )
{
}

Allocate::~Allocate()
{
   for( FIFO *f : allocated_fifo )
   {
      delete( f );
   }
}

void
Allocate::waitTillReady()
{
   while( ! ready );
}

void
Allocate::setReady()
{
   ready = true;
}

void
Allocate::initialize( PortInfo * const src,
                      PortInfo * const dst,
                      FIFO * const fifo )
{
   assert( fifo != nullptr );
   assert( dst  != nullptr );
   assert( src  != nullptr );
   if( src->getFIFO() != nullptr )
   {
      throw PortDoubleInitializeException(
         "Source port \"" + 
            src->my_kernel->output.getPortName( src->my_name ) + 
                "\" already initialized!" );
   }
   if( dst->getFIFO() !=  nullptr )
   {
      throw PortDoubleInitializeException(
         "Destination port \"" + 
            dst->my_kernel->input.getPortName( dst->my_name ) + 
                " already initialized!" );
   }
   src->setFIFO( fifo );
   dst->setFIFO( fifo );
   /** NOTE: this list simply speeds up the monitoring if we want it **/
   allocated_fifo.insert( fifo );
}


void
Allocate::allocate( PortInfo &a, PortInfo &b, void *data )
{
   UNUSED( data );
   FIFO *fifo( nullptr );
   auto &func_map( a.const_map[ Type::Heap ] );
   auto test_func( (*func_map)[ false ] );

   if( a.existing_buffer != nullptr )
   {
      fifo = test_func( a.nitems,
                        a.start_index,
                        a.existing_buffer );
   }
   else
   {
      /** if fixed buffer size, use that, else use INITIAL_ALLOC_SIZE **/
      const auto alloc_size( 
         a.fixed_buffer_size != 0 ? a.fixed_buffer_size : INITIAL_ALLOC_SIZE 
      );
      fifo = test_func( alloc_size            /* items */,
                        ALLOC_ALIGN_WIDTH     /* align */,
                        nullptr );
   }
   initialize( &a, &b, fifo );
   return;
}
