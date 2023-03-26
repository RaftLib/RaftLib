/**
 * graphtools.hpp -
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
#ifndef RAFTGRAPHTOOLS_HPP
#define RAFTGRAPHTOOLS_HPP  1
#include <algorithm>
#include <functional>
#include <unordered_set>
#include <queue>
#include <stack>
#include <vector>

#include "kernel.hpp"
#include "common.hpp"
#include "portmap_t.hpp"
#include "portexception.hpp"
#include "port_info.hpp"

/**
 * edge_func - function to implement if you want to
 * use any of the pre-defined graph tool functions
 * for BFS or DFS.  The void* pointer as the last
 * parameter can be used for pretty much any purpose
 * it will be passed to the function every time it
 * calls.
 */
using edge_func = std::function< void( PortInfo&,
                                       PortInfo&,
                                       void* ) >;
/**
 * vertex_func - function to implement if you
 * want to use the native graph traversal functions.
 * if you use the void* pointer, it comes from the
 * traversal function invocation and will the the
 * same across calls (i.e., it saves state).
 */
using vertex_func = std::function< void( raft::kernel*,
                                         void* ) >;


class GraphTools
{
private:
   /**
    * BFS - breadth first search helper function, performs
    * the actual work for the above BFS functions.  The
    * first variable (queue or stack) is the visiting
    * queue that performs the actual ordering (queue for
    * BFS and stack for the DFS).  The first parameter should
    * come filled with all the source vertices to start the
    * process off.  The set should be empty and contains
    * the vertices that have already been visited.
    * @param q/stack - container with source kernels
    * @param s       - all
    */
   //FIXME - you need to finish this
   static void __BFS( std::queue< raft::kernel* > &queue,
                      std::unordered_set< raft::kernel* > &visited_set,
                      edge_func func,
                      void *data,
                      bool connected_error )
   {
       while( queue.size() > 0 )
       {
           auto *k( queue.front() );
           queue.pop();
           if( k == nullptr ){ break; };
           /** iterate over all out-edges **/
           /** 1) get lock **/
           while( ! k->output.portmap.mutex_map.try_lock() )
           {
               std::this_thread::yield();
           }
           //we have lock, continue
           /** 2) get map **/
           auto &map_of_ports( k->output.portmap.map );
           for( auto &port : map_of_ports )
           {
               PortInfo &source( port.second );
               /** get dst edge to call function on **/
               if( source.other_kernel != nullptr )
               {
                   PortInfo &dst(
                           source.other_kernel->input.getPortInfoFor(
                               source.other_name ) );
                   func( source, dst, data );
               }
               else
               if( connected_error )
               {
                   assert( false );
                   std::stringstream ss;
                   ss << "Unconnected port detected at " <<
                       common::printClassName( *k ) <<
                       "[ \"" <<
                       source.my_name <<
                       " \"], please fix and recompile.";
                   k->output.portmap.mutex_map.unlock();
                   throw PortException( ss.str() );
               }
               /** if the dst kernel hasn't been visited, visit it **/
               if( visited_set.find( source.other_kernel ) ==
                       visited_set.end() )
               {
                   queue.push( source.other_kernel );
                   visited_set.insert( source.other_kernel );
               }
           }
           k->output.portmap.mutex_map.unlock();
       }
       return;
   }

   static void __BFS( std::queue< raft::kernel* > &queue,
                      std::unordered_set< raft::kernel* > &visited_set,
                      vertex_func func,
                      void *data )
   {
       while( queue.size() > 0 )
       {
           auto *source( queue.front() );
           if( source == nullptr ) break;
           queue.pop();
           /** iterate over all out-edges **/
           /** 1) get lock **/
           while( ! source->output.portmap.mutex_map.try_lock() )
           {
               std::this_thread::yield();
           }
           /** 2) get map **/
           auto &map_of_ports( source->output.portmap.map );
           /** 3) visit kernel **/
           func( source, data );
           /** 4) add children to queue **/
           for( auto &port : map_of_ports )
           {
               PortInfo &source( port.second );
               /** get dst edge to call function on **/
               if( source.other_kernel != nullptr  )
               {
                   /** if the dst kernel hasn't been visited, visit it **/
                   if( visited_set.find( source.other_kernel ) ==
                           visited_set.end() )
                   {
                       queue.push( source.other_kernel );
                       visited_set.insert( source.other_kernel );
                   }
               }
           }
           source->output.portmap.mutex_map.unlock();
       }
       return;
   }

public:
   GraphTools() = delete;

   /**
    * BFS - perform a breadth first search of the
    * graph given by 'source_kernels'.  The function
    * 'func' matches the typedef above and is
    * called on each edge of the graph exactly once.
    * For state between calls, the user can define
    * a data struct and pass it via the void ptr data
    * which is passed to the func.
    * @param source_kernels - set of source kernels.
    * @param func - edge_func, funciton to be called
    * @param data - void*, data struct for persistent state
    * @param connected_error, throw an error if not connected
    * NOTE: CONTAINER should be either vector< raft::kernel* >
    * or unordered_set< raft::kernel* >
    */
   template<class CONTAINER>
   static void BFS( CONTAINER &source_kernels,
                    edge_func func,
                    void *data = nullptr,
                    bool connected_error = false )
   {
       std::unordered_set< raft::kernel* > visited_set;
       std::queue< raft::kernel* > queue;
       std::for_each( source_kernels.begin(),
                      source_kernels.end(),
                      [&]( raft::kernel *k )
                      {
                          queue.push( k );
                          visited_set.insert( k );
                      } );
       __BFS( queue, visited_set, func, data, connected_error );
   }

   /**
    * BFS - perform a breadth first search of the
    * graph given by 'source_kernels'.  The function
    * 'func' matches the typedef above and is
    * called on each vertex of the graph exactly once.
    * For state between calls, the user can define
    * a data struct and pass it via the void ptr data
    * which is passed to the func.
    * @param source_kernels - set of source kernels.
    * @param func - vertex_func, funciton to be called
    * @param data - void*, data struct for persistent state
    * NOTE: CONTAINER should be either vector< raft::kernel* >
    * or unordered_set< raft::kernel* >
    */
   template<class CONTAINER>
   static void BFS( CONTAINER &source_kernels,
                    vertex_func func,
                    void *data )
   {
       std::queue< raft::kernel* >   queue;
       std::unordered_set< raft::kernel* >     visited_set;
       std::for_each( source_kernels.begin(),
                      source_kernels.end(),
                      [&]( raft::kernel * k )
                      {
                          queue.push( k );
                          visited_set.insert( k );
                      });

       __BFS( queue, visited_set, func, data );
       return;
   }
};
#endif /* END RAFTGRAPHTOOLS_HPP */
