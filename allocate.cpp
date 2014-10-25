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

Allocate::Allocate( Map &map, volatile bool &exit_alloc ) : 
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
Allocate::initialize( PortInfo *src, PortInfo *dst, FIFO *fifo )
{
   assert( fifo != nullptr );
   if( src != nullptr )
   {
      if( src->getFIFO() != nullptr )
      {
      throw PortDoubleInitializeException( 
         "Source port \"" + src->my_name + "\" already initialized!" );
      }
   }
   if( dst != nullptr && dst->getFIFO() != nullptr )
   {
      throw PortDoubleInitializeException( 
         "Destination port \"" + dst->my_name +  "\" already initialized!" );
   }
   if( src != nullptr )
   {
      src->setFIFO( fifo );
   }
   if( dst != nullptr )
   {
      dst->setFIFO( fifo );
   }
   /** NOTE: this list simply speeds up the monitoring if we want it **/
   allocated_fifo.insert( fifo ); 
}
