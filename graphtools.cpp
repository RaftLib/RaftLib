/**
 * graphtools.cpp - 
 * @author: Jonathan Beard
 * @version: Sat Sep 20 13:15:09 2014
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
#include <algorithm>
#include <set>
#include <queue>
#include <utility>
#include <string>

#include "graphtools.hpp"
#include "port_info.hpp"
#include "kernel.hpp"

void
GraphTools::BFS( std::set< Kernel* > &source_kernels,
                 edge_func func,
                 bool      connected_error )
{
   std::set< Kernel* > visited_set;
   std::queue< Kernel* >     queue;
   std::for_each( source_kernels.begin(),
                  source_kernels.end(),
                  [&]( Kernel *k ){ queue.push( k ); } );
   while( queue.size() > 0 )
   {
      auto * const k( queue.front() );
      queue.pop();
      /** mark current kernel as visited **/
      visited_set.insert( k ); 
      /** iterate over all out-edges **/
      auto &map_of_ports( k->output.portmap );
      for( const std::pair< std::string, PortInfo > &pair : map_of_ports )
      {
         const PortInfo &source( pair.second );
         /** get dst edge to call function on **/
         if( source.other_kernel != nullptr  )
         {
            const PortInfo &dst( source.other_kernel->input.getPortInfoFor( source.other_name ) );
            func( source, dst );
         }
         /** if the dst kernel hasn't been visited, visit it **/
         if( visited_set.find( source.other_kernel ) == visited_set.end() )
         {
            queue.push( source.other_kernel );
         }
      }
   }
   return;
}
