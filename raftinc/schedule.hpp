/**
 * schedule.hpp - 
 * @author: Jonathan Beard
 * @version: Thu Sep 11 15:42:28 2014
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
#ifndef RAFTSCHEDULE_HPP
#define RAFTSCHEDULE_HPP  1
#include <setjmp.h>
#include "signalvars.hpp"
#include "systemsignalhandler.hpp"
#include "rafttypes.hpp"
#include <set>
#include "kernelkeeper.tcc"
#include "defs.hpp"

namespace raft {
   class kernel;
   class map;
}

class Schedule
{
public:

    /** 
     * Schedule - base constructor takes a map object
     * so that all sub-classes can access some of the 
     * map features through the schedule sub-class
     * accessors.
     * @param   map - Map&
     */
    Schedule( raft::map &map );
    
    /**
     * destructor, takes care of cleanup
     */
    virtual ~Schedule();
    
    /**
     * start - called to start execution of all
     * kernels.  Implementation specific so it
     * is purely virtual.
     */
    virtual void start() = 0;


    /** 
     * init - call to pre-process all kernels, this function
     * is called by the map object befure calling start.
     * Default version does absolutely nothing, but is 
     * a shell in the source so that not all derived schedulers
     * must subsequently implement it. 
     */
    virtual void init();
    
    /**
     * kernelRun - all the logic necessary to run a single
     * kernel successfully.  Any additional signal handling
     * should be handled by this function as its the only
     * one that will be universally called by the scheduler.
     * @param   kernel - raft::kernel *const object, non-null kernel
     * @param   finished - volatile bool - function sets to 
     * true when done.
     * @return  true if run with no need for jmp_buf, false if 
     * the scheduler needs to run again with the kernel_state
     */
    static bool kernelRun( raft::kernel * const kernel,
                           volatile bool       &finished );

    /**
     * scheduleKernel - adds the kernel "kernel" to the
     * schedule, ensures that it is run.  Other than
     * that there are no guarantees for its execution.
     * The base version should do for most, however feel
     * free to re-implement in derived class as long as
     * the source_kernels has all of the source kernels, 
     * dst_kernels has all of the destination kernels, and
     * kernel_set has all of the kernels. Before
     * you drop in a kernel, it better be ready to go..all
     * allocations should be complete.
     * @param kernel - raft::kernel*
     */
    virtual void schedule_kernel( raft::kernel * const kernel );
    
    /**
     * terminus_complete - allows the map function to wait
     * till all kernels have signaled to the scheduler that 
     * execution is complete. We'll modify this at some point
     * so that programmers can have a specific barrier, but
     * for now this will get us started. 
     * @return - true when all complete, does not block. 
     */
    bool terminus_complete();


    /**
     * reset_streams - reset all streams within the defined
     * graph so that they're in a state where they can be 
     * re-used, basically undoing the logic that is used
     * for shutting down the dataflow graph when no data
     * is coming. 
     * @return void. 
     */
    virtual void reset_streams();


protected:
   virtual void handleSchedule( raft::kernel * const kernel ) = 0; 

   void signal_complete();
   
   static void invalidateOutputPorts( raft::kernel *kernel );

   static void revalidateOutputPorts( raft::kernel *kernel );

   /** 
    * kernelHasInputData - check each input port for available
    * data, returns true if any of the input ports has available
    * data.
    * @param kernel - raft::kernel
    * @return bool  - true if input data available.
    */
   static bool kernelHasInputData( raft::kernel *kernel );
   
   /**
    * kernelHasNoInputPorts - pretty much exactly like the 
    * function name says, if the param kernel has no valid
    * input ports (this function assumes that kernelHasInputData()
    * has been called and returns false before this function 
    * is called) then it returns true.
    * @params   kernel - raft::kernel*
    * @return  bool   - true if no valid input ports avail
    */
   static bool kernelHasNoInputPorts( raft::kernel *kernel );

   
   /**
    * setPtrSets - add the tracking object from the
    * sub-class to track input/output ptrs in flight 
    * by the kernel. The structure will be the same
    * across all in and output FIFOs, since from within
    * a "kernel" each fifo is accessed with sequential
    * consistency we won't need any fancy locking structures.
    * on each kernel firing, these structures should be
    * checked to see if any allocated objects (the only
    * ones to be put in these sets, need to be "garbage
    * collected"
    * @param kernel - raft::kernel* the one we're registering
    * @param in     - set_t*, the input set
    * @param out    - set_t*, the output set
    * @return void
    */
   static void setPtrSets( raft::kernel * const kernel,
                           ptr_map_t    * const in,
                           ptr_set_t    * const out,
                           ptr_set_t    * const peekset );

   
   static void fifo_gc( ptr_map_t * const in,
                        ptr_set_t * const out,
                        ptr_set_t * const peekset );
  

   /** kernel set **/
   kernelkeeper &kernel_set;
   kernelkeeper &source_kernels;      
   kernelkeeper &dst_kernels;
   kernelkeeper &internally_created_kernels;
   
   bool         complete    = false;
};
#endif /* END RAFTSCHEDULE_HPP */
