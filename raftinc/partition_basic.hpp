/**
 * partition_basic.hpp - the most simple of partitioning algorithms.
 * invoked in two cases, if there are fewer or equal to kernels
 * than cores...or if we simply have no other partitioning scheme 
 * (library) available. 
 *
 * @author: Jonathan Beard
 * @version: Fri Mar 20 08:53:12 2015
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
#ifndef RAFTPARTITION_BASIC_TCC
#define RAFTPARTITION_BASIC_TCC  1
#include <cstdint>
#include <cstddef>
#include "interface_partition.hpp"
#include "schedule.hpp"
#include <utility>
#include "kernel.hpp"

class partition_basic : public interface_partition
{
public:
    partition_basic() = default;
    virtual ~partition_basic() = default;

   /**
    * simple - partiion the kernels in kernelkeeper
    * using a simple partitioning scheme evenly as
    * possible amongst the given run_containers 
    * in run_container.
    * @param    c - kernelkeeper&
    */
   virtual void partition( kernelkeeper &c );
};

#endif /* END RAFTPARTITION_BASIC_TCC */
