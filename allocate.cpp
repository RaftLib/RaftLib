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

#include "allocate.hpp"
#include "port_info.hpp"
#include "map.hpp"
#include "portexception.hpp"

Allocate::Allocate( Map *map ) : source_kernels( map->source_kernels ),
                                 all_kernels(    map->all_kernels )
{
}

Allocate::~Allocate()
{
}

void
Allocate::initialize( PortInfo &src, PortInfo &dst, FIFO *fifo )
{
   assert( fifo != nullptr );
   if( src.fifo )
   {
      throw PortDoubleInitializeException( 
         "Port \"" + src.my_name + "\" already initialized!" );
   }
   if( dst.fifo )
   {
      throw PortDoubleInitializeException( 
         "Port \"" + dst.my_name +  "\" already initialized!" );
   }
   src.fifo = fifo;
   dst.fifo = fifo;
}
