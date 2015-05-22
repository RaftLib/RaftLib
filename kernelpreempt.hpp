/**
 * kernelpreempt.hpp - 
 * @author: Jonathan Beard
 * @version: Fri May 22 15:26:59 2015
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
#ifndef _KERNELPREEMPT_HPP_
#define _KERNELPREEMPT_HPP_  1
namespace raft
{
   class kernel;
}
class kernel_preempt
{
public:
   /** 
    * setRunningState - set the running state of this kernel
    * before preemption so that the scheduler can return 
    * right where this kernel is supposed to be executing.
    * @param k - raft::kernel*
    * @return  - returns flag from setjmp, see man setjmp for 
    *  details
    */
   static std::int32_t setRunningState( raft::kernel * const k );
   /**
    * setPreemptState - set the state to before this kernel
    * was scheduled so that other kernels can run, should
    * be called by the scheduler before executing.  The kernel
    * if it is stuck will call longjmp( state, 1 ) so that
    * the scheduler will receive a 1 from a preempted kernel
    * @param k - raft::kernel*const
    * @return  - returns flag from setjmp, see man setjmp for 
    *  details
    */
   static std::int32_t setPreemptState( raft::kernel * const k );
   /**
    * preempt - called by the kernel if it is stuck
    * and cannot make forward proress.  Will longjmp
    * back to the scheduler and allow another kernel
    * a chance to execute.  The scheduler will receive
    * a 1 in return.
    * @param   k - raft::kernel * const
    */
   static void preempt( raft::kernel * const k );

   /**
    * restore - called by the scheduler to restore the
    * state of this kernel before it was pre-empted so 
    * that progress can continue unabated.  
    * @param k - raft::kernel * const
    */
   static void restore( raft::kernel * const k );
};
#endif /* END _KERNELPREEMPT_HPP_ */
