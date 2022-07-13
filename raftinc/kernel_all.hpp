/**
 * kernel_all.hpp - 
 * @author: Jonathan Beard
 * @version: Sun May 20 09:24:12 2018
 * 
 * Copyright 2018 Jonathan Beard
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
#ifndef RAFTKERNEL_ALL_HPP
#define RAFTKERNEL_ALL_HPP  1

#include "kernel.hpp"

namespace raft
{
class kernel_all : public raft::kernel
{
public:
    kernel_all() : raft::kernel()
    {
        sched_behav = raft::all_port;
    }

    virtual ~kernel_all() = default;

};

} /** end namespace raft **/
#endif /* END RAFTKERNEL_ALL_HPP */
