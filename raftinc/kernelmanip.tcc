/**
 * kernelmanip.tcc - 
 * @author: Jonathan Beard
 * @version: Tue Apr  6 06:38:06 2021
 * 
 * Copyright 2021 Jonathan Beard
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
#ifndef KERNELMANIP_TCC
#define KERNELMANIP_TCC  1

namespace parallel
{

enum type : manip_vec_t { 
    system = 0  /** do whatever the runtime wants, I don't care  **/,
    thread      /** specify a thread for each kernel **/, 
    pool        /** thread pool, one kernel thread per core, many kernels in each **/, 
    process     /** open a new process from this point **/,
    PARALLEL_N };    
} /** end namespace parallel **/ 

/** raft::vm **/
namespace vm
{

enum type : manip_vec_t { 
    flat = parallel::PARALLEL_N       /** not yet implemented, likely using segment  **/, 
    standard                          /** threads share VM space, processes have sep **/, 
    partition                         /** partition graph at this point into a 
                                        * new VM space, platform dependent **/,
    VM_N
}; 
} /** end namespace vm **/
#endif /* END KERNELMANIP_TCC */
