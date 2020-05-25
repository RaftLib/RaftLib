/*
 * partition_scotch.tcc - 
 * @author: Jonathan Beard
 * @version: Tue Mar 10 13:23:12 2015
 * 
 * Copyright 2015 Jonathan Beard
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
#ifndef PARTITION_SCOTCH_TCC
#define PARTITION_SCOTCH_TCC  1
#ifdef USE_PARTITION
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstdint>
#include <functional>
#include <thread>
#include "graph.tcc"
#include "port_info.hpp"
#include "interface_partition.hpp"

class partition_scotch : public interface_partition
{
using container_type = typename kernelkeeper::value_type;

public:
   partition_scotch() = default;

   virtual void partition( kernelkeeper &keeper );

private:
   using weight_function_t = 
      typename std::function< weight_t( PortInfo&,PortInfo&,void* ) >;
   
   using raftgraph_t = raft::graph< edge_id_t, weight_t >;
   
   void get_graph_info( container_type      &c,
                        raftgraph_t         &raft_graph,
                        weight_function_t   weight_func,
                        void                *weight_data );
   
   void  
   simple( container_type    &c, 
           const core_id_t   cores );
   
   /** 
    * simple case check
    */
   bool simple_check( container_type   &c, 
                      const core_id_t   cores );
   
   /**
    * run scotch 
    */
   void run_scotch( container_type   &c,
                    const core_id_t   cores,
                    weight_function_t weight_func,
                    void              *weight );
};
#endif
#endif /* END _PARTITION_HPP_ */
