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
#include <vector>
/** pre-declarations for below **/
struct PortInfo;
namespace raft
{
   class kernel;
}

/**
 * edge_func - function to implement if you want to
 * use any of the pre-defined graph tool functions
 * for BFS or DFS.  The void* pointer as the last
 * parameter can be used for pretty much any purpose
 * it will be passed to the function every time it 
 * calls.
 */
typedef std::function< void(  PortInfo&,  
                              PortInfo&, 
                              void* ) > edge_func;

class GraphTools
{
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
    */
   static void BFS( std::set< raft::kernel* > &source_kernels,  
                    edge_func func,
                    void *data = nullptr,
                    bool connected_error = false );
   
   
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
    */
   static void BFS( std::vector< raft::kernel* > &source_kernels,  
                    edge_func func,
                    void *data = nullptr,
                    bool connected_error = false );
private:
   static void __BFS( std::queue< raft::kernel* > &q,
                      std::set<   raft::kernel* > &s,
                      edge_func                   func,
                      void                        *data,
                      bool                        connected_error );
};
#endif /* END _GRAPHTOOLS_HPP_ */
