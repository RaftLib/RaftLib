/**
 * port_info_types.hpp - 
 * @author: Jonathan Beard
 * @version: Mon Sep 22 20:01:43 2014
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
#ifndef _PORT_INFO_TYPES_HPP_
#define _PORT_INFO_TYPES_HPP_  1
#include <cstddef>
#include <map>
#include <functional>

namespace raft
{
   /* defined in ./parallelk.hpp */
   class parallel_k;
}
/* defined in fifo/fifo.hpp */
class FIFO;

/**
 * instr_map_t - used as a factory function to create type-appropriate queues within
 * the PortInfo object.
 */
using instr_map_t =  std::map< bool, std::function< FIFO* ( std::size_t /** n_items **/,
                                                            std::size_t /** alignof **/,
                                                            void*   /** data struct **/ ) > >;

using split_factory_t = std::function< raft::parallel_k*() >;
using join_factory_t  = std::function< raft::parallel_k*() >;
#endif /* END _PORT_INFO_TYPES_HPP_ */
