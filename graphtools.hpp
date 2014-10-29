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

namespace raft{
   class  kernel;
}
struct PortInfo;

typedef std::function< void(  PortInfo&,  PortInfo& ) > edge_func;

class GraphTools
{
public:
   GraphTools() = delete;
   
   static void BFS( std::set< raft::kernel* > &source_kernels,  
                    edge_func func,
                    bool connected_error = false );
};
#endif /* END _GRAPHTOOLS_HPP_ */
