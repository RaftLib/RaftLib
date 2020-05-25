/**
 * interface_partition.hpp - interface for all the kernel
 * partitioners within RaftLib
 * @author: Jonathan Beard
 * @version: Tue May  3 12:43:22 2016
 * 
 * Copyright 2016 Jonathan Beard
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
#ifndef INTERFACE_PARTITION_HPP
#define INTERFACE_PARTITION_HPP  1

#include "kernelkeeper.tcc"
#include "kernel.hpp"

class interface_partition
{
public:
    interface_partition() = default;
    virtual ~interface_partition() = default;

    /**
     * partition - call me to activate the 
     * partitioning algorithm. The kernel
     * objects are all that you should need.
     * there's a core assignment variable
     * that can map back via the run-time
     * to the hardware type if need be.
     * @param kernels - kernelkeeper&
     */
    virtual void partition( kernelkeeper &keeper ) = 0;

protected:
    /** TODO: add std::enable_if **/
    template < class T, class CORE > 
    static inline void setCore( T &kernel, const CORE core )
    {
        kernel.setCore( core );
        return;
    }
};
#endif /* END INTERFACE_PARTITION_HPP */
