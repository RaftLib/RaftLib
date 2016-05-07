/**
 * partition_dummy.hpp - this class is a dummy partitioner
 * as the name imples for platforms that have no thread 
 * pinning capability.
 *
 * @author: Jonathan Beard
 * @version: Wed May  4 19:27:05 2016
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
#ifndef _PARTITION_DUMMY_HPP_
#define _PARTITION_DUMMY_HPP_  1
#include "interface_partition.hpp"

class partition_dummy : public interface_partition
{
public:
    partition_dummy() = default;
    virtual ~partition_dummy() = default;

    virtual void partition( kernelkeeper &kernels );

};
#endif /* END _PARTITION_DUMMY_HPP_ */
