/**
 * raftmanip.hpp - 
 * @author: Jonathan Beard
 * @version: Tue Aug 16 09:56:56 2016
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
#ifndef _RAFTMANIP_HPP_
#define _RAFTMANIP_HPP_  1

namespace raft
{
    namespace parallel
    {
        enum type { idc     /** do whatever the runtime wants, I don't care  **/,
                    thread  /** specify a thread for each kernel **/, 
                    pool    /** thread pool, one kernel thread per core, many kernels in each **/, 
                    process /** open a new process from this point **/ };
    }
    namespace vm
    {
        enum type { flat        /** not yet implemented **/, 
                    standard    /** threads share VM space, processes have sep **/, 
                    partition   /** partition graph at this point into a new VM space, platform dependent **/ }; 
    }
} /** end namespace raft **/

#endif /* END _RAFTMANIP_HPP_ */
