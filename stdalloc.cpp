/**
 * stdalloc.cpp - 
 * @author: Jonathan Beard
 * @version: Sat Sep 20 19:56:49 2014
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
#include "stdalloc.hpp"
#include "graphtools.hpp"
#include "port_info.hpp"
#include "ringbuffertype.hpp"

stdalloc::stdalloc( Map &map ) : Allocate( map )
{
}

stdalloc::~stdalloc()
{
}

void
stdalloc::run()
{
   auto alloc_func = []( PortInfo &a, PortInfo &b )
   {
      assert( a.type == b.type );
      /** assume everyone needs a heap for the moment to get working **/
      auto *fifo( a.const_map[ RingBufferType::Heap ]->[ false ]( 512 /** hard code buffer size at first **/ ) );
      assert( fifo != nullptr );
      a.fifo = fifo;
      b.fifo = fifo;
   };
   GraphTools::BFS( (this)->source_kernels,
                    alloc_func );
   /** NOTE: we'll keep this thread running in future versions to dynamically update buffer size **/
   return;
}
