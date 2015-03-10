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
#include <sstream>
#include <cxxabi.h>
#include <mutex>

#include "portmap_t.hpp"
#include "portexception.hpp"
#include "graphtools.hpp"
#include "port_info.hpp"
#include "kernel.hpp"

void
GraphTools::BFS( std::set< raft::kernel* > &source_kernels,
                 edge_func func,
                 void      *data,
                 bool      connected_error )
{
   std::set< raft::kernel* > visited_set;
   std::queue< raft::kernel* >     queue;
   std::for_each( source_kernels.begin(),
                  source_kernels.end(),
                  [&]( raft::kernel *k )
                  { 
                     queue.push( k );
                     visited_set.insert( k );
                  } );
   while( queue.size() > 0 )
   {
      auto *k( queue.front() );
      queue.pop();
      /** iterate over all out-edges **/
      /** 1) get lock **/
      std::lock_guard< std::mutex > lock( k->output.portmap.map_mutex );
      /** 2) get map **/
      std::map< std::string, PortInfo > &map_of_ports( k->output.portmap.map );
      for( auto &port : map_of_ports )
      {
         PortInfo &source( port.second );
         /** get dst edge to call function on **/
         if( source.other_kernel != nullptr  )
         {
            PortInfo &dst( 
               source.other_kernel->input.getPortInfoFor( source.other_name ) );
            func( source, dst, data );
         }
         else
         if( connected_error )
         {
            int status( 0 );
            std::stringstream ss;
            ss << "Unconnected port detected at " << 
               abi::__cxa_demangle( typeid( *k ).name(), 0, 0, &status ) << 
                  "[ \"" <<
                  source.my_name << " \"], please fix and recompile.";
            throw PortException( ss.str() );
         }
         /** if the dst kernel hasn't been visited, visit it **/
         if( visited_set.find( source.other_kernel ) == visited_set.end() )
         {
            queue.push( source.other_kernel );
            visited_set.insert( source.other_kernel ); 
         }
      }
   }
   return;
}
