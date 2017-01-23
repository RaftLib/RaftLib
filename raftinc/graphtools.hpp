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
#ifndef _GRAPHTOOLS_HPP_
#define _GRAPHTOOLS_HPP_  1
#include <functional>
#include <set>
#include <queue>
#include <stack>
#include <vector>


/** pre-declarations for below **/
struct PortInfo;
namespace raft
{
   class kernel;
   class temp_map;
}

/**
 * edge_func - function to implement if you want to
 * use any of the pre-defined graph tool functions
 * for BFT or DFT.  The void* pointer as the last
 * parameter can be used for pretty much any purpose
 * it will be passed to the function every time it 
 * calls.
 */
using edge_func = std::function< void(  PortInfo&,  
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
public:
    enum graph_direction : std::uint8_t {   input = 0,
                                            output };
    GraphTools() = delete;
    
    /**
     * BFT - perform a breadth first search of the
     * graph given by 'source_kernels'.  The function
     * 'func' matches the typedef above and is 
     * called on each edge of the graph exactly once.
     * For state between calls, the user can define
     * a data struct and pass it via the void ptr data
     * which is passed to the func.
     * @param source_kernels - set of source kernels.
     * @param func - edge_func, funciton to be called
     * @param direction - direction for the traversal 
     * @param data - void*, data struct for persistent state
     * @param connected_error, throw an error if not connected
     */
    static void BFT( std::set< raft::kernel* > &source_kernels,  
                     edge_func                  func,
                     const graph_direction      direction,
                     void                      *data = nullptr,
                     bool                       connected_error = false );
    
    
    /**
     * BFT - perform a breadth first search of the
     * graph given by 'source_kernels'.  The function
     * 'func' matches the typedef above and is 
     * called on each edge of the graph exactly once.
     * For state between calls, the user can define
     * a data struct and pass it via the void ptr data
     * which is passed to the func.
     * @param source_kernels - set of source kernels.
     * @param func - edge_func, funciton to be called
     * @param direction - direction for the traversal 
     * @param data - void*, data struct for persistent state
     * @param connected_error, throw an error if not connected
     */
    static void BFT( std::vector< raft::kernel* > &source_kernels,  
                     edge_func func,
                     const graph_direction      direction,
                     void *data = nullptr,
                     bool connected_error = false );
     


    static void
    BFT( std::set< raft::kernel* > &source_kernels,
         vertex_func                 func,
         const graph_direction       direction,
         void                        *data );


    /**
     * duplicateFromVertexToSource - take the kernel start
     * as a parameter, an duplicate from its input ports
     * through the conected extent of the graph as far as
     * possible (likely through source but doesn't have
     * to be, this is intended for graph fragments. The
     * return type is a raft::temp_map object which holds
     * pointers to all the kernels, start and source with 
     * respect to this start and the copied sources. This
     * object is not intended to live long (hence the name)
     * @param start - raft::kernel * const, where to start
     *                the duplication from.
     */
    static raft::temp_map*
    duplicateFromVertexToSource( raft::kernel * const start );
    /**
     * duplicateFromVertexToSink - take the kernel start
     * as a parameter, an duplicate from its output ports
     * through the conected extent of the graph as far as
     * possible (likely through terminal destination  but 
     * doesn't have to be, this is intended for graph fragments. 
     * The return type is a raft::temp_map object which holds
     * pointers to all the kernels, start and destination with 
     * respect to this start and the copied destinations. This
     * object is not intended to live long (hence the name)
     * @param start - raft::kernel * const, where to start
     *                the duplication from.
     */
    static raft::temp_map*
    duplicateFromVertexToSink( raft::kernel * const start );

private:
   /**
    * BFT - breadth first search helper function, performs
    * the actual work for the above BFT functions.  The
    * first variable (queue or stack) is the visiting 
    * queue that performs the actual ordering (queue for 
    * BFT and stack for the DFT).  The first parameter should
    * come filled with all the source vertices to start the
    * process off.  The set should be empty and contains
    * the vertices that have already been visited.
    * @param q/stack - container with source kernels
    * @param s       - all
    */
    //FIXME - you need to finish this
   static void __BFT( std::queue< raft::kernel* > &q,
                      std::set<   raft::kernel* > &s,
                      edge_func                   func,
                     const graph_direction      direction,
                      void                        *data,
                      bool                        connected_error );
   static void __BFT( std::queue< raft::kernel* > &q,
                      std::set<   raft::kernel* > &s,
                      vertex_func                 func,
                     const graph_direction      direction,
                      void                        *data );
   static void __DFT( std::stack< raft::kernel* > &stack,
                      std::set<   raft::kernel* > &visited_set,
                      edge_func                   func,
                      void                        *data );
   static void __DFT( std::stack< raft::kernel* > &stack,
                      std::set<   raft::kernel* > &visited_set,
                      vertex_func                 func,
                      void                        *data );
};
#endif /* END _GRAPHTOOLS_HPP_ */
